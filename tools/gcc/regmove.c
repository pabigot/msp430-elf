/* Move registers around to reduce number of move instructions needed.
   Copyright (C) 1987-2013 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */


/* This module makes some simple RTL code transformations which
   improve the subsequent register allocation.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "tm_p.h"
#include "insn-config.h"
#include "recog.h"
#include "target.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "flags.h"
#include "function.h"
#include "expr.h"
#include "basic-block.h"
#include "except.h"
#include "diagnostic-core.h"
#include "reload.h"
#include "tree-pass.h"
#include "df.h"
#include "ira.h"
#include "obstack.h"
#include "ggc.h"
#include "optabs.h"

static int optimize_reg_copy_1 (rtx, rtx, rtx);
static void optimize_reg_copy_2 (rtx, rtx, rtx);
static void optimize_reg_copy_3 (rtx, rtx, rtx);
static void copy_src_to_dest (rtx, rtx, rtx);

enum match_use
{
  READ,
  WRITE,
  READWRITE
};

struct match {
  int with[MAX_RECOG_OPERANDS];
  enum match_use use[MAX_RECOG_OPERANDS];
  int commutative[MAX_RECOG_OPERANDS];
  int early_clobber[MAX_RECOG_OPERANDS];
};

static int find_matches (rtx, struct match *);
static int fixup_match_2 (rtx, rtx, rtx, rtx);

#ifdef AUTO_INC_DEC
struct related;
struct rel_use_chain;
struct rel_mod;
struct rel_use;

static struct rel_use * lookup_related (int, enum reg_class, HOST_WIDE_INT, int);
static struct rel_use * create_rel_use (rtx, rtx *, int, int, int);
static void rel_build_chain            (struct rel_use *, struct rel_use *, struct related *);
static int  recognize_related_for_insn (rtx, int, int);
static void record_reg_use             (rtx *, rtx, int, int);
static void new_reg_use                (rtx, rtx *, int, int, int, int);
static void rel_record_mem             (rtx *, rtx, int, int, int, rtx, int, int);
static void new_base                   (rtx, rtx, int, int);
static void invalidate_related         (rtx, rtx, int, int);
static void find_related               (rtx *, rtx, int, int);
static void find_related_toplev        (rtx, int, int);
static int  chain_starts_earlier       (const void *, const void *);
static int  chain_ends_later           (const void *, const void *);
static int  mod_before                 (const void *, const void *);
static void remove_setting_insns       (struct related *, rtx);
static rtx  perform_addition           (struct rel_mod *, struct rel_use *, rtx, struct rel_use_chain *);
static void modify_address             (struct rel_mod *, struct rel_use *, HOST_WIDE_INT);
static void count_sets                 (rtx, rtx, void *);
#endif
/* Return nonzero if registers with CLASS1 and CLASS2 can be merged without
   causing too much register allocation problems.  */
static int
regclass_compatible_p (reg_class_t class0, reg_class_t class1)
{
  return (class0 == class1
	  || (reg_class_subset_p (class0, class1)
	      && ! targetm.class_likely_spilled_p (class0))
	  || (reg_class_subset_p (class1, class0)
	      && ! targetm.class_likely_spilled_p (class1)));
}


#ifdef AUTO_INC_DEC

/* Some machines have two-address-adds and instructions that can
   use only register-indirect addressing and auto_increment, but no
   offsets.  If multiple fields of a struct are accessed more than
   once, cse will load each of the member addresses in separate registers.
   This not only costs a lot of registers, but also of instructions,
   since each add to initialize an address register must be really expanded
   into a register-register move followed by an add.
   regmove_optimize uses some heuristics to detect this case; if these
   indicate that this is likely, optimize_related_values is run once for
   the entire function.

   We build chains of uses of related values that can be satisfied with the
   same base register by taking advantage of auto-increment address modes
   instead of explicit add instructions.

   We try to link chains with disjoint lifetimes together to reduce the
   number of temporary registers and register-register copies.

   This optimization pass operates on basic blocks one at a time; it could be
   extended to work on extended basic blocks or entire functions.  */

/* For each set of values related to a common base register, we use a
   hash table which maps constant offsets to instructions.

   The instructions mapped to are those that use a register which may,
   (possibly with a change in addressing mode) differ from the initial
   value of the base register by exactly that offset after the
   execution of the instruction.
   Here we define the size of the hash table, and the hash function to use.  */
#define REL_USE_HASH_SIZE 43
#define REL_USE_HASH(I) ((I) % (unsigned HOST_WIDE_INT) REL_USE_HASH_SIZE)

/* For each register in a set of registers that are related, we keep a
   struct related.

   BASE points to the struct related of the base register (i.e. the one
   that was the source of the first three-address add for this set of
   related values).

   INSN is the instruction that initialized the register, or, for the
   base, the instruction that initialized the first non-base register.

   BASE is the register number of the base register.

   For the base register only, the member BASEINFO points to some extra data.

   'luid' here means linear uid.  We count them starting at the function
   start; they are used to avoid overlapping lifetimes.

   UPDATES is a list of instructions that set the register to a new
   value that is still related to the same base.

   When a register in a set of related values is set to something that
   is not related to the base, INVALIDATE_LUID is set to the luid of
   the instruction that does this set.  This is used to avoid re-using
   this register in an overlapping liftime for a related value.

   DEATH is first used to store the insn (if any) where the register dies.
   When the optimization is actually performed, the REG_DEAD note from
   the insn denoted by DEATH is removed.
   Thereafter, the removed death note is stored in DEATH, marking not
   only that the register dies, but also making the note available for reuse.

   We also use a struct related to keep track of registers that have been
   used for anything that we don't recognize as related values.
   The only really interesting datum for these is u.last_luid, which is
   the luid of the last reference we have seen.  These struct relateds
   are marked by a zero INSN field; most other members are not used and
   remain uninitialized.  */

struct related
{
  rtx                       insn;
  rtx                       reg;
  struct related *          base;
  HOST_WIDE_INT             offset;
  struct related *          prev;
  struct update *           updates;
  struct related_baseinfo * baseinfo;
  int                       invalidate_luid;
  rtx                       death;
  int                       reg_orig_calls_crossed;
  int                       reg_set_call_tally;
};

/* HASHTAB maps offsets to register uses with a matching MATCH_OFFSET.
   PREV_BASE points to the struct related for the previous base register
   that we currently keep track of.
   INSN_LUID is the luid of the instruction that started this set of
   related values.  */
struct related_baseinfo
{
  struct rel_use *       hashtab[REL_USE_HASH_SIZE];
  struct rel_use_chain * chains;
  struct related *       prev_base;
  int                    insn_luid;
};

/* INSN is an instruction that sets a register that previously contained
   a related value to a new value that is related to the same base register.
   When the optimization is performed, we have to delete INSN.
   DEATH_INSN points to the insn (if any) where the register died that we
   set in INSN.  When we perform the optimization, the REG_DEAD note has
   to be removed from DEATH_INSN.
   PREV points to the struct update that pertains to the previous
   instruction pertaining to the same register that set it from one
   related value to another one.  */
struct update
{
  rtx             insn;
  rtx             death_insn;
  struct update * prev;
};

struct rel_use_chain
{
  /* Points to first use in this chain.  */
  struct rel_use *        uses;
  struct rel_use_chain *  prev;
  struct rel_use_chain *  linked;
  /* The following fields are only set after the chain has been completed:  */
  /* Last use in this chain.  */
  struct rel_use *        end;
  int                     start_luid;
  int                     end_luid;
  int                     calls_crossed;
  /* The register allocated for this chain.  */
  rtx                     reg;
  /* The death note for this register.  */
  rtx                     death_note;
  /* Offset after execution of last insn.  */
  HOST_WIDE_INT           match_offset;
};

/* ADDRP points to the place where the actual use of the related value is.
   This is commonly a memory address, and has to be set to a register
   or some auto_inc addressing of this register.
   But ADDRP is also used for all other uses of related values to
   the place where the register is inserted; we can tell that an
   unardorned register is to be inserted because no offset adjustment
   is required, hence this is handled by the same logic as register-indirect
   addressing.  The only exception to this is when SET_IN_PARALLEL is set,
   see below.

   OFFSET is the offset that is actually used in this instance, i.e.
   the value of the base register when the set of related values was
   created plus OFFSET yields the value that is used.
   This might be different from the value of the used register before
   executing INSN if we elected to use pre-{in,de}crement addressing.
   If we have the option to use post-{in,de}crement addressing, all
   choices are linked cyclically together with the SIBLING field.
   Otherwise, it's a one-link-cycle, i.e. SIBLING points at the
   struct rel_use it is a member of.

   MATCH_OFFSET is the offset that is available after the execution
   of INSN.  It is the same as OFFSET for straight register-indirect
   addressing and for pre-{in,de}crement addressing, while it differs
   for the post-{in,de}crement addressing modes.

   If SET_IN_PARALLEL is set, MATCH_OFFSET differs from OFFSET, yet
   this is no post-{in,de}crement addressing.  Rather, it is a set
   inside a PARALLEL that adds some constant to a register that holds
   one value of a set of related values that we keep track of.

   NEXT_CHAIN is the link in a chain of rel_use structures.  If nonzero,
   we will ignore this rel_use in a hash table lookup, since it has
   already been appended to.  This field can point to its containing
   rel_use; this means that we found a reason not to append to this
   chain anymore (e.g. if a use comes with a clobber).

   ADDRP then points only to the set destination of this set; another
   struct rel_use is used for the source of the set.

   NO_LINK_PRED is nonzero for the last use in a chain if it cannot be
   the predecessor for a another chain to be linked to.  This can happen
   for uses that come with a clobber, and for uses by a register that
   is live at the end of the processed range of insns (usually a basic
   block).  */

struct rel_use
{
  rtx                insn;
  rtx *              addrp;
  int                luid;
  int                call_tally;
  enum reg_class     _class;
  HOST_WIDE_INT      offset;
  HOST_WIDE_INT      match_offset;
  struct rel_use *   next_chain;
  struct rel_use **  prev_chain_ref;
  struct rel_use *   next_hash;
  struct rel_use *   sibling;
  unsigned int       set_in_parallel : 1;
  unsigned int       no_link_pred    : 1;
};

/* Describe a modification we have to do to the rtl when doing the
   related value optimization.
   There are two types of modifications: emitting a new add or move
   insn, or updating an address within an existing insn.  We distinguish
   between these two cases by testing whether the INSN field is nonzero.  */
struct rel_mod
{
  /* Nonzero if we have to emit a new addition before this insn.
     Otherwise, this describes an address update.  */
  rtx insn;
  /* The chain which this modification belongs to.  */
  struct rel_use_chain * chain;
  /* The position within the insn stream.  Used for sorting the set of
     modifications in ascending order.  */
  int luid;
  /* Used to make the sort stable.  */
  int count;
  /* If this structure describes an addition, this is nonzero if the
     source register is the base reg.  */
  unsigned int from_base : 1;
};

struct related ** regno_related;
struct related *  rel_base_list;
struct related *  unrelatedly_used;

#define rel_alloc(N) obstack_alloc (& related_obstack, (N))
#define rel_new(X)   ((X) = (__typeof__ (X)) rel_alloc (sizeof * (X)))

static struct obstack related_obstack;

/* For each integer machine mode, the minimum and maximum constant that
   can be added with a single constant.
   This is supposed to define an interval around zero; if there are
   singular points disconnected from this interval, we want to leave
   them out.  */
   
static HOST_WIDE_INT add_limits[NUM_MACHINE_MODES][2];

/* Try to find a related value with offset OFFSET from the base
   register belonging to REGNO, using a register with preferred class
   that is compatible with CLASS.  LUID is the insn in which we want
   to use the matched register; this is used to avoid returning a
   match that is an autoincrement within the same insn.  */

static struct rel_use *
lookup_related (int             regno,
		enum reg_class  _class,
		HOST_WIDE_INT   offset,
		int             luid)
{
  struct related * base  = regno_related[regno]->base;
  int              hash  = REL_USE_HASH (offset);
  struct rel_use * match = base->baseinfo->hashtab[hash];
  
  for (; match; match = match->next_hash)
    {
      if (offset != match->match_offset)
	continue;

      /* If MATCH is an autoincrement in the same insn, ensure that it
	 will not be used; otherwise we can end up with invalid rtl
	 that uses the register outside the autoincrement.  */
      if (match->luid == luid && match->offset != match->match_offset)
	continue;

      /* We are looking for a use which we can append to, so ignore
	 anything that has already been appended to, and anything that
	 must terminate a chain for other reasons.  */
      if (match->next_chain)
	continue;

      if (regclass_compatible_p (_class, match->_class))
	break;
    }

  return match;
}

/* Add NEW_USE at the end of the chain that currently ends with MATCH;
   If MATCH is not set, create a new chain.
   BASE is the base register number the chain belongs to.  */

static void
rel_build_chain (struct rel_use * new_use,
		 struct rel_use * match,
		 struct related * base)
{
  int hash;

  if (match)
    {
      struct rel_use * sibling = match;

      do
	{
	  sibling->next_chain = new_use;

	  if (sibling->prev_chain_ref)
	    * sibling->prev_chain_ref = match;

	  sibling = sibling->sibling;
	}
      while (sibling != match);

      new_use->prev_chain_ref = &match->next_chain;
    }
  else
    {
      struct rel_use_chain * new_chain;

      rel_new (new_chain);
      new_chain->uses = new_use;
      new_use->prev_chain_ref = &new_chain->uses;
      new_chain->linked = 0;
      new_chain->prev = base->baseinfo->chains;
      base->baseinfo->chains = new_chain;
    }
  new_use->next_chain = 0;

  hash = REL_USE_HASH (new_use->offset);
  new_use->next_hash = base->baseinfo->hashtab[hash];
  base->baseinfo->hashtab[hash] = new_use;
}

static struct rel_use *
create_rel_use (rtx insn, rtx * xp, int regno, int luid, int call_tally)
{
  struct rel_use * new_use;
  HOST_WIDE_INT    offset = regno_related[regno]->offset;
  enum reg_class   _class = reg_preferred_class (regno);

  rel_new (new_use);
  new_use->insn = insn;
  new_use->addrp = xp;
  new_use->luid = luid;
  new_use->call_tally = call_tally;
  new_use->_class = _class;
  new_use->set_in_parallel = 0;
  new_use->offset = offset;
  new_use->match_offset = offset;
  new_use->sibling = new_use;
  new_use->no_link_pred = 0;

  return new_use;
}

/* Record a new use of reg REGNO, which is found at address XP in INSN.
   LUID and CALL_TALLY correspond to INSN.

   There is a special case for uses of REGNO that must be preserved and
   can't be optimized.  This case can happen either if we reach the end
   of a block and a register which we track is still live, or if we find
   a use of that register that can't be replaced inside an insn.  In
   either case, TERMINATE should be set to a nonzero value.  */

static void
new_reg_use (rtx   insn,
	     rtx * xp,
	     int   regno,
	     int   luid,
	     int   call_tally,
	     int   terminate)
{
  struct rel_use * new_use;
  struct rel_use * match;
  HOST_WIDE_INT    offset = regno_related[regno]->offset;
  enum reg_class   _class = reg_preferred_class (regno);
  struct related * base   = regno_related[regno]->base;

  new_use = create_rel_use (insn, xp, regno, luid, call_tally);
  match = lookup_related (regno, _class, offset, luid);

  rel_build_chain (new_use, match, base);
  if (terminate)
    new_use->next_chain = new_use;
}

/* Record the use of register ADDR in a memory reference.
   ADDRP is the memory location where the address is stored.
   MEM_MODE is the mode of the enclosing MEM.
   SIZE is the size of the memory reference.
   PRE_OFFS is the offset that has to be added to the value in ADDR
   due to PRE_{IN,DE}CREMENT addressing in the original address; likewise,
   POST_OFFSET denotes POST_{IN,DE}CREMENT addressing.  INSN is the
   instruction that uses this address, LUID its luid, and CALL_TALLY
   the current number of calls encountered since the start of the
   function.  */

static GTY(()) rtx test_addr;

static void
rel_record_mem (rtx * addrp,
		rtx   addr,
		int   size,
		int   pre_offs,
		int   post_offs,
		rtx   insn,
		int   luid,
		int   call_tally)
{
  rtx              orig_addr = *addrp;
  int              regno;
  struct related * base;
  HOST_WIDE_INT    offset;
  struct rel_use * new_use;
  struct rel_use * match;
  int              hash;

  if (! REG_P (addr))
    abort ();
  
  regno = REGNO (addr);

  if (! regno_related[regno] || regno_related[regno]->invalidate_luid)
    {
      invalidate_related (addr, insn, luid, call_tally);
      return;
    }

  offset = regno_related[regno]->offset += pre_offs;
  base = regno_related[regno]->base;

  if (base == 0)
    return;

  if (! test_addr)
    test_addr = gen_rtx_PLUS (Pmode, addr, const0_rtx);

  XEXP (test_addr, 0) = addr;
  * addrp = test_addr;

  new_use = create_rel_use (insn, addrp, regno, luid, call_tally);

  match = lookup_related (regno, new_use->_class, offset, luid);

  /* Skip all the autoinc stuff if we found a match within the same insn.  */
  if (match && match->luid == luid)
    goto found_match;

  if (! match)
    {
      /* We can choose PRE_{IN,DE}CREMENT on the spot with the information
	 we have gathered about the preceding instructions, while we have
	 to record POST_{IN,DE}CREMENT possibilities so that we can check
	 later if we have a use for their output value.  */
      /* We use recog here directly because we are only testing here if
	 the changes could be made, but don't really want to make a
	 change right now.  The caching from recog_memoized would only
	 get in the way.  */

      if (HAVE_PRE_INCREMENT)
	{
	  match = lookup_related (regno, new_use->_class, offset - size, luid);
	  PUT_CODE (test_addr, PRE_INC);

	  if (match
	      && match->luid != luid
	      && recog (PATTERN (insn), insn, NULL) >= 0)
	    goto found_match;
	}

      if (HAVE_PRE_DECREMENT)
	{
	  match = lookup_related (regno, new_use->_class, offset + size, luid);
	  PUT_CODE (test_addr, PRE_DEC);

	  if (match
	      && match->luid != luid
	      && recog (PATTERN (insn), insn, NULL) >= 0)
	    goto found_match;
	}

      match = 0;
    }

  PUT_CODE (test_addr, POST_INC);
      
  if (HAVE_POST_INCREMENT && recog (PATTERN (insn), insn, NULL) >= 0)
    {
      struct rel_use * inc_use;

      rel_new (inc_use);
      * inc_use = * new_use;
      inc_use->sibling = new_use;
      new_use->sibling = inc_use;
      inc_use->prev_chain_ref = NULL;
      inc_use->next_chain = NULL;
      hash = REL_USE_HASH (inc_use->match_offset = offset + size);
      inc_use->next_hash = base->baseinfo->hashtab[hash];
      base->baseinfo->hashtab[hash] = inc_use;
    }

  PUT_CODE (test_addr, POST_DEC);

  if (HAVE_POST_DECREMENT && recog (PATTERN (insn), insn, NULL) >= 0)
    {
      struct rel_use * dec_use;

      rel_new (dec_use);
      * dec_use = * new_use;
      dec_use->sibling = new_use->sibling;
      new_use->sibling = dec_use;
      dec_use->prev_chain_ref = NULL;
      dec_use->next_chain = NULL;
      hash = REL_USE_HASH (dec_use->match_offset = offset + size);
      dec_use->next_hash = base->baseinfo->hashtab[hash];
      base->baseinfo->hashtab[hash] = dec_use;
    }

 found_match:
      
  rel_build_chain (new_use, match, base);
  * addrp = orig_addr;

  regno_related[regno]->offset += post_offs;
}

/* Note that REG is set to something that we do not regognize as a
   related value, at an insn with linear uid LUID.  */

static void
invalidate_related (rtx reg, rtx insn, int luid, int call_tally)
{
  int              regno = REGNO (reg);
  struct related * rel = regno_related[regno];

  if (rel && rel->base)
    {
      rel->invalidate_luid = luid;
      rel->reg_orig_calls_crossed = call_tally - rel->reg_set_call_tally;
    }

  if (! rel || rel->base)
    {
      rel_new (rel);
      regno_related[regno] = rel;
      rel->prev = unrelatedly_used;
      unrelatedly_used = rel;
      rel->reg = reg;
      rel->base = NULL;
    }

  rel->invalidate_luid = luid;
  rel->insn = insn;
}

/* Record REG as a new base for related values.  INSN is the insn in which
   we found it, LUID is its luid, and CALL_TALLY the number of calls seen
   up to this point.  */

static void
new_base (rtx reg, rtx insn, int luid, int call_tally)
{
  int              regno = REGNO (reg);
  struct related * new_related;

  rel_new (new_related);
  new_related->reg = reg;
  new_related->insn = insn;
  new_related->updates = 0;
  new_related->reg_set_call_tally = call_tally;
  new_related->base = new_related;
  new_related->offset = 0;
  new_related->prev = 0;
  new_related->invalidate_luid = 0;
  new_related->death = NULL_RTX;
  rel_new (new_related->baseinfo);
  memset ((char *) new_related->baseinfo, 0, sizeof *new_related->baseinfo);
  new_related->baseinfo->prev_base = rel_base_list;
  rel_base_list = new_related;
  new_related->baseinfo->insn_luid = luid;
  regno_related[regno] = new_related;
}

/* Check out if INSN sets a new related value.  Return nonzero if we could
   handle this insn.  */

static int
recognize_related_for_insn (rtx insn, int luid, int call_tally)
{
  rtx set = single_set (insn);
  rtx src, dst;
  rtx src_reg, src_const;
  int src_regno, dst_regno;
  struct related *new_related;

  /* We don't care about register class differences here, since
     we might still find multiple related values share the same
     class even if it is disjunct from the class of the original
     register.  */

  if (set == 0)
    return 0;

  dst = SET_DEST (set);
  src = SET_SRC (set);

  /* First check that we have actually something like
     (set (reg pseudo_dst) (plus (reg pseudo_src) (const_int))) .  */
  if (GET_CODE (src) == PLUS)
    {
      src_reg = XEXP (src, 0);
      src_const = XEXP (src, 1);
    }
  else if (REG_P (src)
	   && GET_MODE_CLASS (GET_MODE (src)) == MODE_INT)
    {
      src_reg = src;
      src_const = const0_rtx;
    }
  else
    return 0;

  if (! REG_P (src_reg)
      || GET_CODE (src_const) != CONST_INT
      || ! REG_P (dst))
    return 0;

  dst_regno = REGNO (dst);
  src_regno = REGNO (src_reg);

  if (src_regno < FIRST_PSEUDO_REGISTER
      || dst_regno < FIRST_PSEUDO_REGISTER)
    return 0;

  /* Check if this is merely an update of a register with a
     value belonging to a group of related values we already
     track.  */
  if (regno_related[dst_regno] && ! regno_related[dst_regno]->invalidate_luid)
    {
      struct update * new_update;

      /* If the base register changes, don't handle this as a
	 related value.  We can currently only attribute the
	 register to one base, and keep record of one lifetime
	 during which we might re-use the register.  */
      if (! regno_related[src_regno]
	  || regno_related[src_regno]->invalidate_luid
	  || (regno_related[dst_regno]->base
	      != regno_related[src_regno]->base))
	return 0;

      regno_related[dst_regno]->offset
	= regno_related[src_regno]->offset + INTVAL (src_const);
      rel_new (new_update);
      new_update->insn = insn;
      new_update->death_insn = regno_related[dst_regno]->death;
      regno_related[dst_regno]->death = NULL_RTX;
      new_update->prev = regno_related[dst_regno]->updates;
      regno_related[dst_regno]->updates = new_update;

      return 1;
    }

  if (! regno_related[src_regno] || regno_related[src_regno]->invalidate_luid)
    {
      if (src_regno == dst_regno)
	return 0;

      new_base (src_reg, insn, luid, call_tally);
    }
  /* If the destination register has been used since we started
     tracking this group of related values, there would be tricky
     lifetime problems that we don't want to tackle right now.  */
  else if (regno_related[dst_regno]
	   && (regno_related[dst_regno]->invalidate_luid
	       >= regno_related[src_regno]->base->baseinfo->insn_luid))
    return 0;

  rel_new (new_related);
  new_related->reg = dst;
  new_related->insn = insn;
  new_related->updates = 0;
  new_related->reg_set_call_tally = call_tally;
  new_related->base = regno_related[src_regno]->base;
  new_related->offset = regno_related[src_regno]->offset + INTVAL (src_const);
  new_related->invalidate_luid = 0;
  new_related->death = NULL_RTX;
  new_related->prev = regno_related[src_regno]->prev;
  regno_related[src_regno]->prev = new_related;
  regno_related[dst_regno] = new_related;

  return 1;
}

static void
record_reg_use (rtx * xp, rtx insn, int luid, int call_tally)
{
  rtx x = * xp;
  int regno = REGNO (x);

  if (! regno_related[regno])
    {
      rel_new (regno_related[regno]);
      regno_related[regno]->prev = unrelatedly_used;
      unrelatedly_used = regno_related[regno];
      regno_related[regno]->reg = x;
      regno_related[regno]->base = NULL;
      regno_related[regno]->invalidate_luid = luid;
      regno_related[regno]->insn = insn;
    }
  else if (regno_related[regno]->invalidate_luid)
    {
      regno_related[regno]->invalidate_luid = luid;
      regno_related[regno]->insn = insn;
    }
  else
    new_reg_use (insn, xp, regno, luid, call_tally, 0);
}

/* Check the RTL fragment pointed to by XP for related values - that is,
   if any new are created, or if they are assigned new values.  Also
   note any other sets so that we can track lifetime conflicts.
   INSN is the instruction XP points into, LUID its luid, and CALL_TALLY
   the number of preceding calls in the function.  */

static void
find_related (rtx * xp, rtx insn, int luid, int call_tally)
{
  rtx x = * xp;
  enum rtx_code code = GET_CODE (x);
  const char * fmt;
  int i;

  if (code == REG)
    record_reg_use (xp, insn, luid, call_tally);
  else if (code == MEM)
    {
      enum machine_mode mem_mode = GET_MODE (x);
      int size = GET_MODE_SIZE (mem_mode);
      rtx * addrp= & XEXP (x, 0);
      rtx   addr = * addrp;

      switch (GET_CODE (addr))
	{
	case REG:
	  rel_record_mem (addrp, addr, size, 0, 0,
			  insn, luid, call_tally);
	  return;
	case PRE_INC:
	  rel_record_mem (addrp, XEXP (addr, 0), size, size, 0,
			  insn, luid, call_tally);
	  return;
	case POST_INC:
	  rel_record_mem (addrp, XEXP (addr, 0), size, 0, size,
			  insn, luid, call_tally);
	  return;
	case PRE_DEC:
	  rel_record_mem (addrp, XEXP (addr, 0), size, -size, 0,
			  insn, luid, call_tally);
	  return;
	case POST_DEC:
	  rel_record_mem (addrp, XEXP (addr, 0), size, 0, -size,
			  insn, luid, call_tally);
	  return;
	default:
	  break;
	}
    }

  fmt = GET_RTX_FORMAT (code);

  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i --)
    {
      if (fmt[i] == 'e')
	find_related (&XEXP (x, i), insn, luid, call_tally);

      if (fmt[i] == 'E')
	{
	  int j;
	  
	  for (j = 0; j < XVECLEN (x, i); j ++)
	    find_related (&XVECEXP (x, i, j), insn, luid, call_tally);
	}
    }
}

/* Process one insn for optimize_related_values.  INSN is the insn, LUID
   and CALL_TALLY its corresponding luid and number of calls seen so
   far.  */

static void
find_related_toplev (rtx insn, int luid, int call_tally)
{
  int i;

  /* First try to process the insn as a whole.  */
  if (recognize_related_for_insn (insn, luid, call_tally))
    return;

  if (GET_CODE (PATTERN (insn)) == USE
      || GET_CODE (PATTERN (insn)) == CLOBBER)
    {
      rtx * xp = &XEXP (PATTERN (insn), 0);
      int regno;

      if (! REG_P (* xp))
	{
	  find_related (xp, insn, luid, call_tally);
	  return;
	}

      regno = REGNO (* xp);
      if (GET_CODE (PATTERN (insn)) == USE
	  && regno_related[regno]
	  && ! regno_related[regno]->invalidate_luid)
	new_reg_use (insn, xp, regno, luid, call_tally, 1);
      invalidate_related (*xp, insn, luid, call_tally);
      return;
    }

  if (CALL_P (insn)
      && CALL_INSN_FUNCTION_USAGE (insn))
    {
      rtx usage;

      for (usage = CALL_INSN_FUNCTION_USAGE (insn);
	   usage;
	   usage = XEXP (usage, 1))
	find_related (& XEXP (usage, 0), insn, luid, call_tally);
    }

  extract_insn (insn);
  /* Process all inputs.  */
  for (i = 0; i < recog_data.n_operands; i++)
    {
      rtx * loc = recog_data.operand_loc[i];
      rtx op = * loc;

      if (op == NULL)
	continue;

      while (GET_CODE (op) == SUBREG
	     || GET_CODE (op) == ZERO_EXTRACT
	     || GET_CODE (op) == SIGN_EXTRACT
	     || GET_CODE (op) == STRICT_LOW_PART)
	loc = & XEXP (op, 0), op = * loc;

      if (recog_data.operand_type[i] == OP_IN
	  || ! REG_P (op))
	find_related (loc, insn, luid, call_tally);
    }

  /* If we have an OP_IN type operand with match_dups, process those
     duplicates also.  */
  for (i = 0; i < recog_data.n_dups; i++)
    {
      int  opno = recog_data.dup_num[i];
      rtx * loc = recog_data.dup_loc[i];
      rtx    op = * loc;

      while (GET_CODE (op) == SUBREG
	     || GET_CODE (op) == ZERO_EXTRACT
	     || GET_CODE (op) == SIGN_EXTRACT
	     || GET_CODE (op) == STRICT_LOW_PART)
	loc = & XEXP (op, 0), op = * loc;

      if (recog_data.operand_type[opno] == OP_IN
	  || ! REG_P (op))
	find_related (loc, insn, luid, call_tally);
    }

  /* Process outputs.  */
  for (i = 0; i < recog_data.n_operands; i ++)
    {
      enum op_type type = recog_data.operand_type[i];
      rtx * loc = recog_data.operand_loc[i];
      rtx op = * loc;

      if (op == NULL)
	continue;

      while (   GET_CODE (op) == SUBREG
	     || GET_CODE (op) == ZERO_EXTRACT
	     || GET_CODE (op) == SIGN_EXTRACT
	     || GET_CODE (op) == STRICT_LOW_PART)
	loc = & XEXP (op, 0), op = *loc;

      /* Detect if we're storing into only one word of a multiword
	 subreg.  */
      if (loc != recog_data.operand_loc[i] && type == OP_OUT)
	type = OP_INOUT;

      if (REG_P (op))
	{
	  int regno = REGNO (op);

	  if (type == OP_INOUT)
	    {							
	      /* This is a use we can't handle.  Add a dummy use
		 of this register as well as invalidating it.  */
	      if (regno_related[regno]
		  && ! regno_related[regno]->invalidate_luid)
		new_reg_use (insn, loc, regno, luid, call_tally, 1);
	    }

	  if (type != OP_IN)
	    /* A set of a register invalidates it (unless the set was
	       handled by recognize_related_for_insn).  */
	    invalidate_related (op, insn, luid, call_tally);
	}
    }
}

/* Comparison functions for qsort.  */

static int
chain_starts_earlier (const void * chain1, const void * chain2)
{
  int d = (  (*(struct rel_use_chain **)chain2)->start_luid
	   - (*(struct rel_use_chain **)chain1)->start_luid);
  if (! d)
    d = (  (*(struct rel_use_chain **)chain2)->uses->offset
         - (*(struct rel_use_chain **)chain1)->uses->offset);
  if (! d)
    d = (  (*(struct rel_use_chain **)chain2)->uses->set_in_parallel
         - (*(struct rel_use_chain **)chain1)->uses->set_in_parallel);

  /* If set_in_parallel is not set on both chain's first use, they must
     differ in start_luid or offset, since otherwise they would use the
     same chain.
     Thus the remaining problem is with set_in_parallel uses; for these, we
     know that *addrp is a register.  Since the same register may not be set
     multiple times in the same insn, the registers must be different.  */

  if (! d)
    d = (  REGNO (*(*(struct rel_use_chain **)chain2)->uses->addrp)
         - REGNO (*(*(struct rel_use_chain **)chain1)->uses->addrp));
  return d;
}

static int
chain_ends_later (const void * chain1, const void * chain2)
{
  int d = (  (*(struct rel_use_chain **)chain1)->end->no_link_pred
	   - (*(struct rel_use_chain **)chain2)->end->no_link_pred);
  if (! d)
    d = (  (*(struct rel_use_chain **)chain1)->end_luid
	 - (*(struct rel_use_chain **)chain2)->end_luid);
  if (! d)
    d = (  (*(struct rel_use_chain **)chain2)->uses->offset
         - (*(struct rel_use_chain **)chain1)->uses->offset);
  if (! d)
    d = (  (*(struct rel_use_chain **)chain2)->uses->set_in_parallel
         - (*(struct rel_use_chain **)chain1)->uses->set_in_parallel);

  /* If set_in_parallel is not set on both chain's first use, they must
     differ in start_luid or offset, since otherwise they would use the
     same chain.
     Thus the remaining problem is with set_in_parallel uses; for these, we
     know that *addrp is a register.  Since the same register may not be set
     multiple times in the same insn, the registers must be different.  */

  if (! d)
    {
      rtx reg1 = (*(*(struct rel_use_chain **)chain1)->uses->addrp);
      rtx reg2 = (*(*(struct rel_use_chain **)chain2)->uses->addrp);

      switch (GET_CODE (reg1))
	{
	case REG:
	  break;

	case PRE_INC:
	case POST_INC:
	case PRE_DEC:
	case POST_DEC:
	  reg1 = XEXP (reg1, 0);
	  break;

	default:
	  abort ();
	}

      switch (GET_CODE (reg2))
	{
	case REG:
	  break;

	case PRE_INC:
	case POST_INC:
	case PRE_DEC:
	case POST_DEC:
	  reg2 = XEXP (reg2, 0);
	  break;

	default:
	  abort ();
	}

	d = (REGNO (reg2) - REGNO (reg1));
    }
  return d;
}

/* Called through qsort, used to sort rel_mod structures in ascending
   order by luid.  */

static int
mod_before (const void * ptr1, const void * ptr2)
{
  struct rel_mod * insn1 = (struct rel_mod *) ptr1;
  struct rel_mod * insn2 = (struct rel_mod *) ptr2;

  if (insn1->luid != insn2->luid)
    return insn1->luid - insn2->luid;
  /* New add insns get inserted before the luid, modifications are
     performed within this luid.  */
  if (insn1->insn == 0 && insn2->insn != 0)
    return 1;
  if (insn2->insn == 0 && insn1->insn != 0)
    return -1;
  return insn1->count - insn2->count;
}

/* Update REG_N_SETS given a newly generated insn.  Called through
   note_stores.  */

static void
count_sets (rtx x, const_rtx pat ATTRIBUTE_UNUSED, void * data ATTRIBUTE_UNUSED)
{
  if (REG_P (x))
    INC_REG_N_SETS (REGNO (x), 1);
}

/* First pass of performing the optimization on a set of related values:
   remove all the setting insns, death notes and refcount increments that
   are now obsolete.
   INSERT_AFTER is an insn which we must delete by turning it into a note,
   since it is needed later.  */

static void
remove_setting_insns (struct related * rel_base, rtx insert_after)
{
  struct related * rel;

  for (rel = rel_base; rel; rel = rel->prev)
    {
      struct update * update;
      int             regno = REGNO (rel->reg);

      if (rel != rel_base)
	{
	  /* The first setting insn might be the start of a basic block.  */
	  if (rel->insn == rel_base->insn
	      /* We have to preserve insert_after.  */
	      || rel->insn == insert_after)
	    {
	      PUT_CODE (rel->insn, NOTE);
	      NOTE_KIND (rel->insn) = NOTE_INSN_DELETED;
	    }
	  else
	    delete_insn (rel->insn);
	  INC_REG_N_SETS (regno, -1);
	}

      REG_N_CALLS_CROSSED (regno) -= rel->reg_orig_calls_crossed;

      for (update = rel->updates; update; update = update->prev)
	{
	  rtx death_insn = update->death_insn;

	  if (death_insn)
	    {
	      rtx death_note
		= find_reg_note (death_insn, REG_DEAD, rel->reg);

	      if (! death_note)
		death_note
		  = find_reg_note (death_insn, REG_UNUSED, rel->reg);

	      remove_note (death_insn, death_note);
	      REG_N_DEATHS (regno) --;
	    }

	  /* We have to preserve insert_after.  */
	  if (update->insn == insert_after)
	    {
	      PUT_CODE (update->insn, NOTE);
	      NOTE_KIND (update->insn) = NOTE_INSN_DELETED;
	    }
	  else
	    delete_insn (update->insn);

	  INC_REG_N_SETS (regno, -1);
	}

      if (rel->death)
	{
	  rtx death_note = find_reg_note (rel->death, REG_DEAD, rel->reg);

	  if (! death_note)
	    death_note = find_reg_note (rel->death, REG_UNUSED, rel->reg);

	  remove_note (rel->death, death_note);
	  rel->death = death_note;
	  REG_N_DEATHS (regno) --;
	}
    }
}

/* Create a new add (or move) instruction as described by the modification
   MOD, which is for the rel_use USE.  BASE_REG is the base register for
   this set of related values, REL_BASE_REG_USER is the chain that uses
   it.  */

static rtx
perform_addition (struct rel_mod *       mod,
		  struct rel_use *       use,
		  rtx                    base_reg,
		  struct rel_use_chain * rel_base_reg_user)
{
  HOST_WIDE_INT use_offset = use->offset;
  /* We have to generate a new addition or move insn and emit it
     before the current use in this chain.  */
  HOST_WIDE_INT new_offset = use_offset;
  rtx           reg = mod->chain->reg;
  rtx           src_reg;

  if (mod->from_base)
    {
      src_reg = base_reg;
      if (rel_base_reg_user)
	use_offset -= rel_base_reg_user->match_offset;
    }
  else
    {
      src_reg = reg;
      use_offset -= mod->chain->match_offset;
    }

  if (use_offset != 0 || src_reg != reg)
    {
      rtx _new;

      if (use_offset == 0)
	_new = gen_move_insn (reg, src_reg);
      else
	_new = gen_add3_insn (reg, src_reg, GEN_INT (use_offset));

      if (! _new)
	abort ();

      if (GET_CODE (_new) == SEQUENCE)
	{
	  int i;

	  for (i = XVECLEN (_new, 0) - 1; i >= 0; i --)
	    note_stores (PATTERN (XVECEXP (_new, 0, i)), count_sets,
			 NULL);
	}
      else
	note_stores (_new, count_sets, NULL);
      _new = emit_insn_before (_new, mod->insn);

      mod->chain->match_offset = new_offset;
      return _new;
    }
  return NULL_RTX;
}

/* Perform the modification described by MOD, which applies to the use
   described by USE.  This function calls validate_change; the caller
   must call apply_change_group after all modifications for the same
   insn have been performed.  */

static void
modify_address (struct rel_mod * mod,
		struct rel_use * use,
		HOST_WIDE_INT    current_offset)
{
  HOST_WIDE_INT use_offset = use->offset;
  rtx reg = mod->chain->reg;
  /* We have to perform a modification on a given use.  The
     current use will be removed from the chain afterwards.  */
  rtx addr = * use->addrp;

  if (! REG_P (addr))
    remove_note (use->insn,
		 find_reg_note (use->insn, REG_INC,
				XEXP (addr, 0)));

  if (use_offset == current_offset)
    {
      if (use->set_in_parallel)
	{
	  INC_REG_N_SETS (REGNO (addr), -1);
	  addr = reg;
	}
      else if (use->match_offset > use_offset)
	addr = gen_rtx_POST_INC (Pmode, reg);
      else if (use->match_offset < use_offset)
	addr = gen_rtx_POST_DEC (Pmode, reg);
      else
	addr = reg;
    }
  else if (use_offset > current_offset)
    addr = gen_rtx_PRE_INC (Pmode, reg);
  else
    addr = gen_rtx_PRE_DEC (Pmode, reg);

  /* Group changes from the same chain for the same insn
     together, to avoid failures for match_dups.  */
  validate_change (use->insn, use->addrp, addr, 1);

  if (addr != reg)
    REG_NOTES (use->insn)
      = gen_rtx_EXPR_LIST (VOIDmode, reg, REG_NOTES (use->insn));

  /* Update the chain's state: set match_offset as appropriate,
     and move towards the next use.  */
  mod->chain->match_offset = use->match_offset;
  mod->chain->uses = use->next_chain;
  if (mod->chain->uses == 0 && mod->chain->linked)
    {
      struct rel_use_chain *linked = mod->chain->linked;
      mod->chain->linked = linked->linked;
      mod->chain->uses = linked->uses;
    }
}
#endif  /* AUTO_INC_DEC */

#ifdef AUTO_INC_DEC

/* Find the place in the rtx X where REG is used as a memory address.
   Return the MEM rtx that so uses it.
   If PLUSCONST is nonzero, search instead for a memory address equivalent to
   (plus REG (const_int PLUSCONST)).

   If such an address does not appear, return 0.
   If REG appears more than once, or is used other than in such an address,
   return (rtx) 1.  */

static rtx
find_use_as_address (rtx x, rtx reg, HOST_WIDE_INT plusconst)
{
  enum rtx_code code = GET_CODE (x);
  const char * const fmt = GET_RTX_FORMAT (code);
  int i;
  rtx value = 0;
  rtx tem;

  if (code == MEM && XEXP (x, 0) == reg && plusconst == 0)
    return x;

  if (code == MEM && GET_CODE (XEXP (x, 0)) == PLUS
      && XEXP (XEXP (x, 0), 0) == reg
      && CONST_INT_P (XEXP (XEXP (x, 0), 1))
      && INTVAL (XEXP (XEXP (x, 0), 1)) == plusconst)
    return x;

  if (code == SIGN_EXTRACT || code == ZERO_EXTRACT)
    {
      /* If REG occurs inside a MEM used in a bit-field reference,
	 that is unacceptable.  */
      if (find_use_as_address (XEXP (x, 0), reg, 0) != 0)
	return (rtx) (size_t) 1;
    }

  if (x == reg)
    return (rtx) (size_t) 1;

  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	{
	  tem = find_use_as_address (XEXP (x, i), reg, plusconst);
	  if (value == 0)
	    value = tem;
	  else if (tem != 0)
	    return (rtx) (size_t) 1;
	}
      else if (fmt[i] == 'E')
	{
	  int j;
	  for (j = XVECLEN (x, i) - 1; j >= 0; j--)
	    {
	      tem = find_use_as_address (XVECEXP (x, i, j), reg, plusconst);
	      if (value == 0)
		value = tem;
	      else if (tem != 0)
		return (rtx) (size_t) 1;
	    }
	}
    }

  return value;
}


/* INC_INSN is an instruction that adds INCREMENT to REG.
   Try to fold INC_INSN as a post/pre in/decrement into INSN.
   Iff INC_INSN_SET is nonzero, inc_insn has a destination different from src.
   Return nonzero for success.  */
static int
try_auto_increment (rtx insn, rtx inc_insn, rtx inc_insn_set, rtx reg,
		    HOST_WIDE_INT increment, int pre)
{
  enum rtx_code inc_code;

  rtx pset = single_set (insn);
  if (pset)
    {
      /* Can't use the size of SET_SRC, we might have something like
	 (sign_extend:SI (mem:QI ...  */
      rtx use = find_use_as_address (pset, reg, 0);
      if (use != 0 && use != (rtx) (size_t) 1)
	{
	  int size = GET_MODE_SIZE (GET_MODE (use));
	  if (0
	      || (HAVE_POST_INCREMENT
		  && pre == 0 && (inc_code = POST_INC, increment == size))
	      || (HAVE_PRE_INCREMENT
		  && pre == 1 && (inc_code = PRE_INC, increment == size))
	      || (HAVE_POST_DECREMENT
		  && pre == 0 && (inc_code = POST_DEC, increment == -size))
	      || (HAVE_PRE_DECREMENT
		  && pre == 1 && (inc_code = PRE_DEC, increment == -size))
	  )
	    {
	      if (inc_insn_set)
		validate_change
		  (inc_insn,
		   &SET_SRC (inc_insn_set),
		   XEXP (SET_SRC (inc_insn_set), 0), 1);
	      validate_change (insn, &XEXP (use, 0),
			       gen_rtx_fmt_e (inc_code,
					      GET_MODE (XEXP (use, 0)), reg),
			       1);
	      if (apply_change_group ())
		{
		  /* If there is a REG_DEAD note on this insn, we must
		     change this not to REG_UNUSED meaning that the register
		     is set, but the value is dead.  Failure to do so will
		     result in sched1 dying -- when it recomputes lifetime
		     information, the number of REG_DEAD notes will have
		     changed.  */
		  rtx note = find_reg_note (insn, REG_DEAD, reg);
		  if (note)
		    PUT_REG_NOTE_KIND (note, REG_UNUSED);

		  add_reg_note (insn, REG_INC, reg);

		  if (! inc_insn_set)
		    delete_insn (inc_insn);
		  return 1;
		}
	    }
	}
    }
  return 0;
}
#endif


static int *regno_src_regno;

/* INSN is a copy from SRC to DEST, both registers, and SRC does not die
   in INSN.

   Search forward to see if SRC dies before either it or DEST is modified,
   but don't scan past the end of a basic block.  If so, we can replace SRC
   with DEST and let SRC die in INSN.

   This will reduce the number of registers live in that range and may enable
   DEST to be tied to SRC, thus often saving one register in addition to a
   register-register copy.  */

static int
optimize_reg_copy_1 (rtx insn, rtx dest, rtx src)
{
  rtx p, q;
  rtx note;
  rtx dest_death = 0;
  int sregno = REGNO (src);
  int dregno = REGNO (dest);
  basic_block bb = BLOCK_FOR_INSN (insn);

  /* We don't want to mess with hard regs if register classes are small.  */
  if (sregno == dregno
      || (targetm.small_register_classes_for_mode_p (GET_MODE (src))
	  && (sregno < FIRST_PSEUDO_REGISTER
	      || dregno < FIRST_PSEUDO_REGISTER))
      /* We don't see all updates to SP if they are in an auto-inc memory
	 reference, so we must disallow this optimization on them.  */
      || sregno == STACK_POINTER_REGNUM || dregno == STACK_POINTER_REGNUM)
    return 0;

  for (p = NEXT_INSN (insn); p; p = NEXT_INSN (p))
    {
      if (! INSN_P (p))
	continue;
      if (BLOCK_FOR_INSN (p) != bb)
	break;

      if (reg_set_p (src, p) || reg_set_p (dest, p)
	  /* If SRC is an asm-declared register, it must not be replaced
	     in any asm.  Unfortunately, the REG_EXPR tree for the asm
	     variable may be absent in the SRC rtx, so we can't check the
	     actual register declaration easily (the asm operand will have
	     it, though).  To avoid complicating the test for a rare case,
	     we just don't perform register replacement for a hard reg
	     mentioned in an asm.  */
	  || (sregno < FIRST_PSEUDO_REGISTER
	      && asm_noperands (PATTERN (p)) >= 0
	      && reg_overlap_mentioned_p (src, PATTERN (p)))
	  /* Don't change hard registers used by a call.  */
	  || (CALL_P (p) && sregno < FIRST_PSEUDO_REGISTER
	      && find_reg_fusage (p, USE, src))
	  /* Don't change a USE of a register.  */
	  || (GET_CODE (PATTERN (p)) == USE
	      && reg_overlap_mentioned_p (src, XEXP (PATTERN (p), 0))))
	break;

      /* See if all of SRC dies in P.  This test is slightly more
	 conservative than it needs to be.  */
      if ((note = find_regno_note (p, REG_DEAD, sregno)) != 0
	  && GET_MODE (XEXP (note, 0)) == GET_MODE (src))
	{
	  int failed = 0;
	  int d_length = 0;
	  int s_length = 0;
	  int d_n_calls = 0;
	  int s_n_calls = 0;
	  int s_freq_calls = 0;
	  int d_freq_calls = 0;

	  /* We can do the optimization.  Scan forward from INSN again,
	     replacing regs as we go.  Set FAILED if a replacement can't
	     be done.  In that case, we can't move the death note for SRC.
	     This should be rare.  */

	  /* Set to stop at next insn.  */
	  for (q = next_real_insn (insn);
	       q != next_real_insn (p);
	       q = next_real_insn (q))
	    {
	      if (reg_overlap_mentioned_p (src, PATTERN (q)))
		{
		  /* If SRC is a hard register, we might miss some
		     overlapping registers with validate_replace_rtx,
		     so we would have to undo it.  We can't if DEST is
		     present in the insn, so fail in that combination
		     of cases.  */
		  if (sregno < FIRST_PSEUDO_REGISTER
		      && reg_mentioned_p (dest, PATTERN (q)))
		    failed = 1;

		  /* Attempt to replace all uses.  */
		  else if (!validate_replace_rtx (src, dest, q))
		    failed = 1;

		  /* If this succeeded, but some part of the register
		     is still present, undo the replacement.  */
		  else if (sregno < FIRST_PSEUDO_REGISTER
			   && reg_overlap_mentioned_p (src, PATTERN (q)))
		    {
		      validate_replace_rtx (dest, src, q);
		      failed = 1;
		    }
		}

	      /* For SREGNO, count the total number of insns scanned.
		 For DREGNO, count the total number of insns scanned after
		 passing the death note for DREGNO.  */
	      if (!DEBUG_INSN_P (p))
		{
		  s_length++;
		  if (dest_death)
		    d_length++;
		}

	      /* If the insn in which SRC dies is a CALL_INSN, don't count it
		 as a call that has been crossed.  Otherwise, count it.  */
	      if (q != p && CALL_P (q))
		{
		  /* Similarly, total calls for SREGNO, total calls beyond
		     the death note for DREGNO.  */
		  s_n_calls++;
		  s_freq_calls += REG_FREQ_FROM_BB  (BLOCK_FOR_INSN (q));
		  if (dest_death)
		    {
		      d_n_calls++;
		      d_freq_calls += REG_FREQ_FROM_BB  (BLOCK_FOR_INSN (q));
		    }
		}

	      /* If DEST dies here, remove the death note and save it for
		 later.  Make sure ALL of DEST dies here; again, this is
		 overly conservative.  */
	      if (dest_death == 0
		  && (dest_death = find_regno_note (q, REG_DEAD, dregno)) != 0)
		{
		  if (GET_MODE (XEXP (dest_death, 0)) != GET_MODE (dest))
		    failed = 1, dest_death = 0;
		  else
		    remove_note (q, dest_death);
		}
	    }

	  if (! failed)
	    {
	      /* These counters need to be updated if and only if we are
		 going to move the REG_DEAD note.  */
	      if (sregno >= FIRST_PSEUDO_REGISTER)
		{
		  if (REG_LIVE_LENGTH (sregno) >= 0)
		    {
		      REG_LIVE_LENGTH (sregno) -= s_length;
		      /* REG_LIVE_LENGTH is only an approximation after
			 combine if sched is not run, so make sure that we
			 still have a reasonable value.  */
		      if (REG_LIVE_LENGTH (sregno) < 2)
			REG_LIVE_LENGTH (sregno) = 2;
		    }

		  REG_N_CALLS_CROSSED (sregno) -= s_n_calls;
		  REG_FREQ_CALLS_CROSSED (sregno) -= s_freq_calls;
		}

	      /* Move death note of SRC from P to INSN.  */
	      remove_note (p, note);
	      XEXP (note, 1) = REG_NOTES (insn);
	      REG_NOTES (insn) = note;
	    }

	  /* DEST is also dead if INSN has a REG_UNUSED note for DEST.  */
	  if (! dest_death
	      && (dest_death = find_regno_note (insn, REG_UNUSED, dregno)))
	    {
	      PUT_REG_NOTE_KIND (dest_death, REG_DEAD);
	      remove_note (insn, dest_death);
	    }

	  /* Put death note of DEST on P if we saw it die.  */
	  if (dest_death)
	    {
	      XEXP (dest_death, 1) = REG_NOTES (p);
	      REG_NOTES (p) = dest_death;

	      if (dregno >= FIRST_PSEUDO_REGISTER)
		{
		  /* If and only if we are moving the death note for DREGNO,
		     then we need to update its counters.  */
		  if (REG_LIVE_LENGTH (dregno) >= 0)
		    REG_LIVE_LENGTH (dregno) += d_length;
		  REG_N_CALLS_CROSSED (dregno) += d_n_calls;
		  REG_FREQ_CALLS_CROSSED (dregno) += d_freq_calls;
		}
	    }

	  return ! failed;
	}

      /* If SRC is a hard register which is set or killed in some other
	 way, we can't do this optimization.  */
      else if (sregno < FIRST_PSEUDO_REGISTER
	       && dead_or_set_p (p, src))
	break;
    }
  return 0;
}

/* INSN is a copy of SRC to DEST, in which SRC dies.  See if we now have
   a sequence of insns that modify DEST followed by an insn that sets
   SRC to DEST in which DEST dies, with no prior modification of DEST.
   (There is no need to check if the insns in between actually modify
   DEST.  We should not have cases where DEST is not modified, but
   the optimization is safe if no such modification is detected.)
   In that case, we can replace all uses of DEST, starting with INSN and
   ending with the set of SRC to DEST, with SRC.  We do not do this
   optimization if a CALL_INSN is crossed unless SRC already crosses a
   call or if DEST dies before the copy back to SRC.

   It is assumed that DEST and SRC are pseudos; it is too complicated to do
   this for hard registers since the substitutions we may make might fail.  */

static void
optimize_reg_copy_2 (rtx insn, rtx dest, rtx src)
{
  rtx p, q;
  rtx set;
  int sregno = REGNO (src);
  int dregno = REGNO (dest);
  basic_block bb = BLOCK_FOR_INSN (insn);

  for (p = NEXT_INSN (insn); p; p = NEXT_INSN (p))
    {
      if (! INSN_P (p))
	continue;
      if (BLOCK_FOR_INSN (p) != bb)
	break;

      set = single_set (p);
      if (set && SET_SRC (set) == dest && SET_DEST (set) == src
	  && find_reg_note (p, REG_DEAD, dest))
	{
	  /* We can do the optimization.  Scan forward from INSN again,
	     replacing regs as we go.  */

	  /* Set to stop at next insn.  */
	  for (q = insn; q != NEXT_INSN (p); q = NEXT_INSN (q))
	    if (INSN_P (q))
	      {
		if (reg_mentioned_p (dest, PATTERN (q)))
		  {
		    rtx note;

		    PATTERN (q) = replace_rtx (PATTERN (q), dest, src);
		    note = FIND_REG_INC_NOTE (q, dest);
		    if (note)
		      {
			remove_note (q, note);
			add_reg_note (q, REG_INC, src);
		      }
		    df_insn_rescan (q);
		  }

		if (CALL_P (q))
		  {
		    int freq = REG_FREQ_FROM_BB  (BLOCK_FOR_INSN (q));
		    REG_N_CALLS_CROSSED (dregno)--;
		    REG_N_CALLS_CROSSED (sregno)++;
		    REG_FREQ_CALLS_CROSSED (dregno) -= freq;
		    REG_FREQ_CALLS_CROSSED (sregno) += freq;
		  }
	      }

	  remove_note (p, find_reg_note (p, REG_DEAD, dest));
	  REG_N_DEATHS (dregno)--;
	  remove_note (insn, find_reg_note (insn, REG_DEAD, src));
	  REG_N_DEATHS (sregno)--;
	  return;
	}

      if (reg_set_p (src, p)
	  || find_reg_note (p, REG_DEAD, dest)
	  || (CALL_P (p) && REG_N_CALLS_CROSSED (sregno) == 0))
	break;
    }
}

/* INSN is a ZERO_EXTEND or SIGN_EXTEND of SRC to DEST.
   Look if SRC dies there, and if it is only set once, by loading
   it from memory.  If so, try to incorporate the zero/sign extension
   into the memory read, change SRC to the mode of DEST, and alter
   the remaining accesses to use the appropriate SUBREG.  This allows
   SRC and DEST to be tied later.  */
static void
optimize_reg_copy_3 (rtx insn, rtx dest, rtx src)
{
  rtx src_reg = XEXP (src, 0);
  int src_no = REGNO (src_reg);
  int dst_no = REGNO (dest);
  rtx p, set, set_insn;
  enum machine_mode old_mode;
  basic_block bb = BLOCK_FOR_INSN (insn);

  if (src_no < FIRST_PSEUDO_REGISTER
      || dst_no < FIRST_PSEUDO_REGISTER
      || ! find_reg_note (insn, REG_DEAD, src_reg)
      || REG_N_DEATHS (src_no) != 1
      || REG_N_SETS (src_no) != 1)
    return;

  for (p = PREV_INSN (insn); p && ! reg_set_p (src_reg, p); p = PREV_INSN (p))
    if (INSN_P (p) && BLOCK_FOR_INSN (p) != bb)
      break;

  if (! p || BLOCK_FOR_INSN (p) != bb)
    return;

  if (! (set = single_set (p))
      || !MEM_P (SET_SRC (set))
      /* If there's a REG_EQUIV note, this must be an insn that loads an
	 argument.  Prefer keeping the note over doing this optimization.  */
      || find_reg_note (p, REG_EQUIV, NULL_RTX)
      || SET_DEST (set) != src_reg)
    return;

  /* Be conservative: although this optimization is also valid for
     volatile memory references, that could cause trouble in later passes.  */
  if (MEM_VOLATILE_P (SET_SRC (set)))
    return;

  /* Do not use a SUBREG to truncate from one mode to another if truncation
     is not a nop.  */
  if (GET_MODE_BITSIZE (GET_MODE (src_reg)) <= GET_MODE_BITSIZE (GET_MODE (src))
      && !TRULY_NOOP_TRUNCATION_MODES_P (GET_MODE (src), GET_MODE (src_reg)))
    return;

  set_insn = p;
  old_mode = GET_MODE (src_reg);
  PUT_MODE (src_reg, GET_MODE (src));
  XEXP (src, 0) = SET_SRC (set);

  /* Include this change in the group so that it's easily undone if
     one of the changes in the group is invalid.  */
  validate_change (p, &SET_SRC (set), src, 1);

  /* Now walk forward making additional replacements.  We want to be able
     to undo all the changes if a later substitution fails.  */
  while (p = NEXT_INSN (p), p != insn)
    {
      if (! INSN_P (p))
	continue;

      /* Make a tentative change.  */
      validate_replace_rtx_group (src_reg,
				  gen_lowpart_SUBREG (old_mode, src_reg),
				  p);
    }

  validate_replace_rtx_group (src, src_reg, insn);

  /* Now see if all the changes are valid.  */
  if (! apply_change_group ())
    {
      /* One or more changes were no good.  Back out everything.  */
      PUT_MODE (src_reg, old_mode);
      XEXP (src, 0) = src_reg;
    }
  else
    {
      rtx note = find_reg_note (set_insn, REG_EQUAL, NULL_RTX);
      if (note)
	{
	  if (rtx_equal_p (XEXP (note, 0), XEXP (src, 0)))
	    {
	      XEXP (note, 0)
		= gen_rtx_fmt_e (GET_CODE (src), GET_MODE (src),
				 XEXP (note, 0));
	      df_notes_rescan (set_insn);
	    }
	  else
	    remove_note (set_insn, note);
	}
    }
}


/* If we were not able to update the users of src to use dest directly, try
   instead moving the value to dest directly before the operation.  */

static void
copy_src_to_dest (rtx insn, rtx src, rtx dest)
{
  rtx seq;
  rtx link;
  rtx next;
  rtx set;
  rtx move_insn;
  rtx *p_insn_notes;
  rtx *p_move_notes;
  int src_regno;
  int dest_regno;

  /* A REG_LIVE_LENGTH of -1 indicates the register must not go into
     a hard register, e.g. because it crosses as setjmp.  See the
     comment in regstat.c:regstat_bb_compute_ri.  Don't try to apply
     any transformations to such regs.  */

  if (REG_P (src)
      && REG_LIVE_LENGTH (REGNO (src)) > 0
      && REG_P (dest)
      && REG_LIVE_LENGTH (REGNO (dest)) > 0
      && (set = single_set (insn)) != NULL_RTX
      && !reg_mentioned_p (dest, SET_SRC (set))
      && GET_MODE (src) == GET_MODE (dest))
    {
      int old_num_regs = reg_rtx_no;

      /* Generate the src->dest move.  */
      start_sequence ();
      emit_move_insn (dest, src);
      seq = get_insns ();
      end_sequence ();
      /* If this sequence uses new registers, we may not use it.  */
      if (old_num_regs != reg_rtx_no
	  || ! validate_replace_rtx (src, dest, insn))
	{
	  /* We have to restore reg_rtx_no to its old value, lest
	     recompute_reg_usage will try to compute the usage of the
	     new regs, yet reg_n_info is not valid for them.  */
	  reg_rtx_no = old_num_regs;
	  return;
	}
      emit_insn_before (seq, insn);
      move_insn = PREV_INSN (insn);
      p_move_notes = &REG_NOTES (move_insn);
      p_insn_notes = &REG_NOTES (insn);

      /* Move any notes mentioning src to the move instruction.  */
      for (link = REG_NOTES (insn); link != NULL_RTX; link = next)
	{
	  next = XEXP (link, 1);
	  if (XEXP (link, 0) == src)
	    {
	      *p_move_notes = link;
	      p_move_notes = &XEXP (link, 1);
	    }
	  else
	    {
	      *p_insn_notes = link;
	      p_insn_notes = &XEXP (link, 1);
	    }
	}

      *p_move_notes = NULL_RTX;
      *p_insn_notes = NULL_RTX;

      /* Update the various register tables.  */
      dest_regno = REGNO (dest);
      INC_REG_N_SETS (dest_regno, 1);
      REG_LIVE_LENGTH (dest_regno)++;
      src_regno = REGNO (src);
      if (! find_reg_note (move_insn, REG_DEAD, src))
	REG_LIVE_LENGTH (src_regno)++;
    }
}

/* reg_set_in_bb[REGNO] points to basic block iff the register is set
   only once in the given block and has REG_EQUAL note.  */

static basic_block *reg_set_in_bb;

/* Size of reg_set_in_bb array.  */
static unsigned int max_reg_computed;


/* Return whether REG is set in only one location, and is set to a
   constant, but is set in a different basic block from INSN (an
   instructions which uses REG).  In this case REG is equivalent to a
   constant, and we don't want to break that equivalence, because that
   may increase register pressure and make reload harder.  If REG is
   set in the same basic block as INSN, we don't worry about it,
   because we'll probably need a register anyhow (??? but what if REG
   is used in a different basic block as well as this one?).  */

static bool
reg_is_remote_constant_p (rtx reg, rtx insn)
{
  basic_block bb;
  rtx p;
  int max;

  if (!reg_set_in_bb)
    {
      max_reg_computed = max = max_reg_num ();
      reg_set_in_bb = XCNEWVEC (basic_block, max);

      FOR_EACH_BB (bb)
	FOR_BB_INSNS (bb, p)
	  {
	    rtx s;

	    if (!INSN_P (p))
	      continue;
	    s = single_set (p);
	    /* This is the instruction which sets REG.  If there is a
	       REG_EQUAL note, then REG is equivalent to a constant.  */
	    if (s != 0
	        && REG_P (SET_DEST (s))
	        && REG_N_SETS (REGNO (SET_DEST (s))) == 1
	        && find_reg_note (p, REG_EQUAL, NULL_RTX))
	      reg_set_in_bb[REGNO (SET_DEST (s))] = bb;
	  }
    }

  gcc_assert (REGNO (reg) < max_reg_computed);
  if (reg_set_in_bb[REGNO (reg)] == NULL)
    return false;
  return (reg_set_in_bb[REGNO (reg)] != BLOCK_FOR_INSN (insn));
}

/* INSN is adding a CONST_INT to a REG.  We search backwards looking for
   another add immediate instruction with the same source and dest registers,
   and if we find one, we change INSN to an increment, and return 1.  If
   no changes are made, we return 0.

   This changes
     (set (reg100) (plus reg1 offset1))
     ...
     (set (reg100) (plus reg1 offset2))
   to
     (set (reg100) (plus reg1 offset1))
     ...
     (set (reg100) (plus reg100 offset2-offset1))  */

/* ??? What does this comment mean?  */
/* cse disrupts preincrement / postdecrement sequences when it finds a
   hard register as ultimate source, like the frame pointer.  */

static int
fixup_match_2 (rtx insn, rtx dst, rtx src, rtx offset)
{
  rtx p, dst_death = 0;
  int length, num_calls = 0, freq_calls = 0;
  basic_block bb = BLOCK_FOR_INSN (insn);

  /* If SRC dies in INSN, we'd have to move the death note.  This is
     considered to be very unlikely, so we just skip the optimization
     in this case.  */
  if (find_regno_note (insn, REG_DEAD, REGNO (src)))
    return 0;

  /* Scan backward to find the first instruction that sets DST.  */

  for (length = 0, p = PREV_INSN (insn); p; p = PREV_INSN (p))
    {
      rtx pset;

      if (! INSN_P (p))
	continue;
      if (BLOCK_FOR_INSN (p) != bb)
	break;

      if (find_regno_note (p, REG_DEAD, REGNO (dst)))
	dst_death = p;
      if (! dst_death && !DEBUG_INSN_P (p))
	length++;

      pset = single_set (p);
      if (pset && SET_DEST (pset) == dst
	  && GET_CODE (SET_SRC (pset)) == PLUS
	  && XEXP (SET_SRC (pset), 0) == src
	  && CONST_INT_P (XEXP (SET_SRC (pset), 1)))
	{
	  HOST_WIDE_INT newconst
	    = INTVAL (offset) - INTVAL (XEXP (SET_SRC (pset), 1));
	  rtx add = gen_add3_insn (dst, dst, GEN_INT (newconst));

	  if (add && validate_change (insn, &PATTERN (insn), add, 0))
	    {
	      /* Remove the death note for DST from DST_DEATH.  */
	      if (dst_death)
		{
		  remove_death (REGNO (dst), dst_death);
		  REG_LIVE_LENGTH (REGNO (dst)) += length;
		  REG_N_CALLS_CROSSED (REGNO (dst)) += num_calls;
		  REG_FREQ_CALLS_CROSSED (REGNO (dst)) += freq_calls;
		}

	      if (dump_file)
		fprintf (dump_file,
			 "Fixed operand of insn %d.\n",
			  INSN_UID (insn));

#ifdef AUTO_INC_DEC
	      for (p = PREV_INSN (insn); p; p = PREV_INSN (p))
		{
		  if (! INSN_P (p))
		    continue;
		  if (BLOCK_FOR_INSN (p) != bb)
		    break;
		  if (reg_overlap_mentioned_p (dst, PATTERN (p)))
		    {
		      if (try_auto_increment (p, insn, 0, dst, newconst, 0))
			return 1;
		      break;
		    }
		}
	      for (p = NEXT_INSN (insn); p; p = NEXT_INSN (p))
		{
		  if (! INSN_P (p))
		    continue;
		  if (BLOCK_FOR_INSN (p) != bb)
		    break;
		  if (reg_overlap_mentioned_p (dst, PATTERN (p)))
		    {
		      try_auto_increment (p, insn, 0, dst, newconst, 1);
		      break;
		    }
		}
#endif
	      return 1;
	    }
	}

      if (reg_set_p (dst, PATTERN (p)))
	break;

      /* If we have passed a call instruction, and the
         pseudo-reg SRC is not already live across a call,
         then don't perform the optimization.  */
      /* reg_set_p is overly conservative for CALL_INSNS, thinks that all
	 hard regs are clobbered.  Thus, we only use it for src for
	 non-call insns.  */
      if (CALL_P (p))
	{
	  if (! dst_death)
	    {
	      num_calls++;
	      freq_calls += REG_FREQ_FROM_BB  (BLOCK_FOR_INSN (p));
	    }

	  if (REG_N_CALLS_CROSSED (REGNO (src)) == 0)
	    break;

	  if ((HARD_REGISTER_P (dst) && call_used_regs [REGNO (dst)])
	      || find_reg_fusage (p, CLOBBER, dst))
	    break;
	}
      else if (reg_set_p (src, PATTERN (p)))
	break;
    }

  return 0;
}

/* A forward pass.  Replace output operands with input operands.  */

static void
regmove_forward_pass (void)
{
  basic_block bb;
  rtx insn;

  if (! flag_expensive_optimizations)
    return;

  if (dump_file)
    fprintf (dump_file, "Starting forward pass...\n");

  FOR_EACH_BB (bb)
    {
      FOR_BB_INSNS (bb, insn)
	{
	  rtx set = single_set (insn);
	  if (! set)
	    continue;

	  if ((GET_CODE (SET_SRC (set)) == SIGN_EXTEND
	       || GET_CODE (SET_SRC (set)) == ZERO_EXTEND)
	      && REG_P (XEXP (SET_SRC (set), 0))
	      && REG_P (SET_DEST (set)))
	    optimize_reg_copy_3 (insn, SET_DEST (set), SET_SRC (set));

	  if (REG_P (SET_SRC (set))
	      && REG_P (SET_DEST (set)))
	    {
	      /* If this is a register-register copy where SRC is not dead,
		 see if we can optimize it.  If this optimization succeeds,
		 it will become a copy where SRC is dead.  */
	      if ((find_reg_note (insn, REG_DEAD, SET_SRC (set))
		   || optimize_reg_copy_1 (insn, SET_DEST (set), SET_SRC (set)))
		  && REGNO (SET_DEST (set)) >= FIRST_PSEUDO_REGISTER)
		{
		  /* Similarly for a pseudo-pseudo copy when SRC is dead.  */
		  if (REGNO (SET_SRC (set)) >= FIRST_PSEUDO_REGISTER)
		    optimize_reg_copy_2 (insn, SET_DEST (set), SET_SRC (set));
		  if (regno_src_regno[REGNO (SET_DEST (set))] < 0
		      && SET_SRC (set) != SET_DEST (set))
		    {
		      int srcregno = REGNO (SET_SRC (set));
		      if (regno_src_regno[srcregno] >= 0)
			srcregno = regno_src_regno[srcregno];
		      regno_src_regno[REGNO (SET_DEST (set))] = srcregno;
		    }
		}
	    }
	}
    }
}

/* A backward pass.  Replace input operands with output operands.  */

static void
regmove_backward_pass (void)
{
  basic_block bb;
  rtx insn, prev;

  if (dump_file)
    fprintf (dump_file, "Starting backward pass...\n");

  FOR_EACH_BB_REVERSE (bb)
    {
      /* ??? Use the safe iterator because fixup_match_2 can remove
	     insns via try_auto_increment.  */
      FOR_BB_INSNS_REVERSE_SAFE (bb, insn, prev)
	{
	  struct match match;
	  rtx copy_src, copy_dst;
	  int op_no, match_no;
	  int success = 0;

	  if (! INSN_P (insn))
	    continue;

	  if (! find_matches (insn, &match))
	    continue;

	  /* Now scan through the operands looking for a destination operand
	     which is supposed to match a source operand.
	     Then scan backward for an instruction which sets the source
	     operand.  If safe, then replace the source operand with the
	     dest operand in both instructions.  */

	  copy_src = NULL_RTX;
	  copy_dst = NULL_RTX;
	  for (op_no = 0; op_no < recog_data.n_operands; op_no++)
	    {
	      rtx set, p, src, dst;
	      rtx src_note, dst_note;
	      int num_calls = 0, freq_calls = 0;
	      enum reg_class src_class, dst_class;
	      int length;

	      match_no = match.with[op_no];

	      /* Nothing to do if the two operands aren't supposed to match.  */
	      if (match_no < 0)
		continue;

	      dst = recog_data.operand[match_no];
	      src = recog_data.operand[op_no];

	      if (!REG_P (src))
		continue;

	      if (!REG_P (dst)
		  || REGNO (dst) < FIRST_PSEUDO_REGISTER
		  || REG_LIVE_LENGTH (REGNO (dst)) < 0
		  || GET_MODE (src) != GET_MODE (dst))
		continue;

	      /* If the operands already match, then there is nothing to do.  */
	      if (operands_match_p (src, dst))
		continue;

	      if (match.commutative[op_no] >= 0)
		{
		  rtx comm = recog_data.operand[match.commutative[op_no]];
		  if (operands_match_p (comm, dst))
		    continue;
		}

	      set = single_set (insn);
	      if (! set)
		continue;

	      /* Note that single_set ignores parts of a parallel set for
		 which one of the destinations is REG_UNUSED.  We can't
		 handle that here, since we can wind up rewriting things
		 such that a single register is set twice within a single
		 parallel.  */
	      if (reg_set_p (src, insn))
		continue;

	      /* match_no/dst must be a write-only operand, and
		 operand_operand/src must be a read-only operand.  */
	      if (match.use[op_no] != READ
		  || match.use[match_no] != WRITE)
		continue;

	      if (match.early_clobber[match_no]
		  && count_occurrences (PATTERN (insn), src, 0) > 1)
		continue;

	      /* Make sure match_no is the destination.  */
	      if (recog_data.operand[match_no] != SET_DEST (set))
		continue;

	      if (REGNO (src) < FIRST_PSEUDO_REGISTER)
		{
		  if (GET_CODE (SET_SRC (set)) == PLUS
		      && CONST_INT_P (XEXP (SET_SRC (set), 1))
		      && XEXP (SET_SRC (set), 0) == src
		      && fixup_match_2 (insn, dst, src,
					XEXP (SET_SRC (set), 1)))
		    break;
		  continue;
		}
	      src_class = reg_preferred_class (REGNO (src));
	      dst_class = reg_preferred_class (REGNO (dst));

	      if (! (src_note = find_reg_note (insn, REG_DEAD, src)))
		{
		  /* We used to force the copy here like in other cases, but
		     it produces worse code, as it eliminates no copy
		     instructions and the copy emitted will be produced by
		     reload anyway.  On patterns with multiple alternatives,
		     there may be better solution available.

		     In particular this change produced slower code for numeric
		     i387 programs.  */

		  continue;
		}

	      if (! regclass_compatible_p (src_class, dst_class))
		{
		  if (!copy_src)
		    {
		      copy_src = src;
		      copy_dst = dst;
		    }
		  continue;
		}

	      /* Can not modify an earlier insn to set dst if this insn
		 uses an old value in the source.  */
	      if (reg_overlap_mentioned_p (dst, SET_SRC (set)))
		{
		  if (!copy_src)
		    {
		      copy_src = src;
		      copy_dst = dst;
		    }
		  continue;
		}

	      /* If src is set once in a different basic block,
		 and is set equal to a constant, then do not use
		 it for this optimization, as this would make it
		 no longer equivalent to a constant.  */

	      if (reg_is_remote_constant_p (src, insn))
		{
		  if (!copy_src)
		    {
		      copy_src = src;
		      copy_dst = dst;
		    }
		  continue;
		}


	      if (dump_file)
		fprintf (dump_file,
			 "Could fix operand %d of insn %d matching operand %d.\n",
			 op_no, INSN_UID (insn), match_no);

	      /* Scan backward to find the first instruction that uses
		 the input operand.  If the operand is set here, then
		 replace it in both instructions with match_no.  */

	      for (length = 0, p = PREV_INSN (insn); p; p = PREV_INSN (p))
		{
		  rtx pset;

		  if (! INSN_P (p))
		    continue;
		  if (BLOCK_FOR_INSN (p) != bb)
		    break;

		  if (!DEBUG_INSN_P (p))
		    length++;

		  /* ??? See if all of SRC is set in P.  This test is much
		     more conservative than it needs to be.  */
		  pset = single_set (p);
		  if (pset && SET_DEST (pset) == src)
		    {
		      /* We use validate_replace_rtx, in case there
			 are multiple identical source operands.  All
			 of them have to be changed at the same time:
			 when validate_replace_rtx() calls
			 apply_change_group().  */
		      validate_change (p, &SET_DEST (pset), dst, 1);
		      if (validate_replace_rtx (src, dst, insn))
			success = 1;
		      break;
		    }

		  /* We can't make this change if DST is mentioned at
		     all in P, since we are going to change its value.
		     We can't make this change if SRC is read or
		     partially written in P, since we are going to
		     eliminate SRC.  However, if it's a debug insn, we
		     can't refrain from making the change, for this
		     would cause codegen differences, so instead we
		     invalidate debug expressions that reference DST,
		     and adjust references to SRC in them so that they
		     become references to DST.  */
		  if (reg_mentioned_p (dst, PATTERN (p)))
		    {
		      if (DEBUG_INSN_P (p))
			validate_change (p, &INSN_VAR_LOCATION_LOC (p),
					 gen_rtx_UNKNOWN_VAR_LOC (), 1);
		      else
			break;
		    }
		  if (reg_overlap_mentioned_p (src, PATTERN (p)))
		    {
		      if (DEBUG_INSN_P (p))
			validate_replace_rtx_group (src, dst, p);
		      else
			break;
		    }

		  /* If we have passed a call instruction, and the
		     pseudo-reg DST is not already live across a call,
		     then don't perform the optimization.  */
		  if (CALL_P (p))
		    {
		      num_calls++;
		      freq_calls += REG_FREQ_FROM_BB  (BLOCK_FOR_INSN (p));

		      if (REG_N_CALLS_CROSSED (REGNO (dst)) == 0)
			break;
		    }
		}

	      if (success)
		{
		  int dstno, srcno;

		  /* Remove the death note for SRC from INSN.  */
		  remove_note (insn, src_note);
		  /* Move the death note for SRC to P if it is used
		     there.  */
		  if (reg_overlap_mentioned_p (src, PATTERN (p)))
		    {
		      XEXP (src_note, 1) = REG_NOTES (p);
		      REG_NOTES (p) = src_note;
		    }
		  /* If there is a REG_DEAD note for DST on P, then remove
		     it, because DST is now set there.  */
		  if ((dst_note = find_reg_note (p, REG_DEAD, dst)))
		    remove_note (p, dst_note);

		  dstno = REGNO (dst);
		  srcno = REGNO (src);

		  INC_REG_N_SETS (dstno, 1);
		  INC_REG_N_SETS (srcno, -1);

		  REG_N_CALLS_CROSSED (dstno) += num_calls;
		  REG_N_CALLS_CROSSED (srcno) -= num_calls;
		  REG_FREQ_CALLS_CROSSED (dstno) += freq_calls;
		  REG_FREQ_CALLS_CROSSED (srcno) -= freq_calls;

		  REG_LIVE_LENGTH (dstno) += length;
		  if (REG_LIVE_LENGTH (srcno) >= 0)
		    {
		      REG_LIVE_LENGTH (srcno) -= length;
		      /* REG_LIVE_LENGTH is only an approximation after
			 combine if sched is not run, so make sure that we
			 still have a reasonable value.  */
		      if (REG_LIVE_LENGTH (srcno) < 2)
			REG_LIVE_LENGTH (srcno) = 2;
		    }

		  if (dump_file)
		    fprintf (dump_file,
			     "Fixed operand %d of insn %d matching operand %d.\n",
			     op_no, INSN_UID (insn), match_no);

		  break;
		}
	      else if (num_changes_pending () > 0)
		cancel_changes (0);
	    }

	  /* If we weren't able to replace any of the alternatives, try an
	     alternative approach of copying the source to the destination.  */
	  if (!success && copy_src != NULL_RTX)
	    copy_src_to_dest (insn, copy_src, copy_dst);
	}
    }
}

/* Main entry for the register move optimization.  */

static unsigned int
regmove_optimize (void)
{
  int i;
  int nregs = max_reg_num ();

  df_note_add_problem ();
  df_analyze ();

  regstat_init_n_sets_and_refs ();
  regstat_compute_ri ();

  if (flag_ira_loop_pressure)
    ira_set_pseudo_classes (true, dump_file);

  regno_src_regno = XNEWVEC (int, nregs);
  for (i = nregs; --i >= 0; )
    regno_src_regno[i] = -1;

  /* A forward pass.  Replace output operands with input operands.  */
  regmove_forward_pass ();

  /* A backward pass.  Replace input operands with output operands.  */
  regmove_backward_pass ();

  /* Clean up.  */
  free (regno_src_regno);
  if (reg_set_in_bb)
    {
      free (reg_set_in_bb);
      reg_set_in_bb = NULL;
    }
  regstat_free_n_sets_and_refs ();
  regstat_free_ri ();
  if (flag_ira_loop_pressure)
    free_reg_info ();
  return 0;
}

/* Returns nonzero if INSN's pattern has matching constraints for any operand.
   Returns 0 if INSN can't be recognized, or if the alternative can't be
   determined.

   Initialize the info in MATCHP based on the constraints.  */

static int
find_matches (rtx insn, struct match *matchp)
{
  int likely_spilled[MAX_RECOG_OPERANDS];
  int op_no;
  int any_matches = 0;

  extract_insn (insn);
  if (! constrain_operands (0))
    return 0;

  /* Must initialize this before main loop, because the code for
     the commutative case may set matches for operands other than
     the current one.  */
  for (op_no = recog_data.n_operands; --op_no >= 0; )
    matchp->with[op_no] = matchp->commutative[op_no] = -1;

  for (op_no = 0; op_no < recog_data.n_operands; op_no++)
    {
      const char *p;
      char c;
      int i = 0;

      p = recog_data.constraints[op_no];

      likely_spilled[op_no] = 0;
      matchp->use[op_no] = READ;
      matchp->early_clobber[op_no] = 0;
      if (*p == '=')
	matchp->use[op_no] = WRITE;
      else if (*p == '+')
	matchp->use[op_no] = READWRITE;

      for (;*p && i < which_alternative; p++)
	if (*p == ',')
	  i++;

      while ((c = *p) != '\0' && c != ',')
	{
	  switch (c)
	    {
	    case '=':
	      break;
	    case '+':
	      break;
	    case '&':
	      matchp->early_clobber[op_no] = 1;
	      break;
	    case '%':
	      matchp->commutative[op_no] = op_no + 1;
	      matchp->commutative[op_no + 1] = op_no;
	      break;

	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	      {
		char *end;
		unsigned long match_ul = strtoul (p, &end, 10);
		int match = match_ul;

		p = end;

		if (match < op_no && likely_spilled[match])
		  continue;
		matchp->with[op_no] = match;
		any_matches = 1;
		if (matchp->commutative[op_no] >= 0)
		  matchp->with[matchp->commutative[op_no]] = match;
	      }
	    continue;

	  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'h':
	  case 'j': case 'k': case 'l': case 'p': case 'q': case 't': case 'u':
	  case 'v': case 'w': case 'x': case 'y': case 'z': case 'A': case 'B':
	  case 'C': case 'D': case 'W': case 'Y': case 'Z':
	    if (targetm.class_likely_spilled_p (REG_CLASS_FROM_CONSTRAINT ((unsigned char) c, p)))
	      likely_spilled[op_no] = 1;
	    break;
	  }
	  p += CONSTRAINT_LEN (c, p);
	}
    }
  return any_matches;
}



static bool
gate_handle_regmove (void)
{
  return (optimize > 0 && flag_regmove);
}


struct rtl_opt_pass pass_regmove =
{
 {
  RTL_PASS,
  "regmove",                            /* name */
  OPTGROUP_NONE,                        /* optinfo_flags */
  gate_handle_regmove,                  /* gate */
  regmove_optimize,			/* execute */
  NULL,                                 /* sub */
  NULL,                                 /* next */
  0,                                    /* static_pass_number */
  TV_REGMOVE,                           /* tv_id */
  0,                                    /* properties_required */
  0,                                    /* properties_provided */
  0,                                    /* properties_destroyed */
  0,                                    /* todo_flags_start */
  TODO_df_finish | TODO_verify_rtl_sharing |
  TODO_ggc_collect                      /* todo_flags_finish */
 }
};
