/* Support for Alpha-based NetApp box.
   Based on code Copyright 1988, 1991, 1992, 1993, 1994 Free Software
   Foundation, Inc.

This file is part of GDB.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "defs.h"
#include <sys/types.h>
#include <sys/param.h>
#include "symtab.h"
#include "inferior.h"

static void netapp_fetch_thread_register (CORE_ADDR sp, int regno);

static void netapp_store_thread_register (CORE_ADDR sp, int regno, char *regs);

/* A PID is just a pointer to a "struct sk_Proc".  Its context is
   in the "context" member of that structure. */

/* A context switch is done by calling "sk_hw_switch()".

   On the Alpha, "sk_hw_switch()" does a context switch is done by:

	Allocating a stack frame
	putting in it all callee-save registers except for SP, in the
	order specified by the Alpha calling sequence (except, on some
	versions of NetApp software, for the callee-save floating point
	registers)
	Storing SP into the "sp" member of the "context" member of the
	    "sk_Proc" structure for the process

   When "sk_hw_switch" is called, its first argument is a pointer
   to the "context" member of the process giving up control, i.e.
   a pointer to the place where the SP of that process will be
   stored.

   Other registers are treated as caller-save registers, so we don't
   care what values that have, which is good because they're not saved
   across a context switch.

   Thus, what we have is:

	SP for process ->   RA for caller of "sk_hw_switch()"
			    other saved registers for process

   so we fetch N pointer's worth of data pointed to by the SP, and get
   the N registers in question from that, and the SP by subtracting
   M from the SP for the process, where N is the size of the stack frame
   divided by the size of a pointer and M is the size of the stack frame.

   This will return a set of registers that make it look as if
   "sk_hw_switch()" had just returned; I guess if this really
   confuses people we can hack it to back the PC up so that it looks
   as if we're just about to call "sk_hw_switch()".  It seems kind of
   pointless to leave "sk_hw_switch()" on the stack, though, unless
   not doing so screws up our ability to do certain things to threads
   other than the currently active one. */

#define NOT_IN_CONTEXT	-1	/* not available for other threads */

static int regmap[NUM_REGS];

static int frame_size;

static void init_from_sk_hw_switch_proc_desc (void);

/* Fetch registers for "sk" process whose PID is "inferior_ptid".
   Must not be called if "inferior_ptid" equals "sk_running_proc",
   as the registers for the running process are not where they'd
   be if the thread weren't running. */
void
netapp_fetch_thread_registers (int regno)
{
  CORE_ADDR sp;

  init_from_sk_hw_switch_proc_desc ();

  /* Get the SP for the process. */
  sp = netapp_get_proc_sp (PIDGET (inferior_ptid)); 

  if (regno >= 0)
    {
      /* Get the register in question. */
      netapp_fetch_thread_register (sp, regno);
    }
  else
    {
      /* Get all the registers. */
      for (regno = 0; regno < NUM_REGS; regno++)
	netapp_fetch_thread_register (sp, regno);
    }
}

static void
netapp_fetch_thread_register (CORE_ADDR sp, int regno)
{
  int i;
  CORE_ADDR regaddr;
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];

  if (regno == SP_REGNUM)
    {
      /* Special case - the SP isn't stored in the context, it points
         to the context, and the SP we want is past the context. */
      store_address (buf, REGISTER_RAW_SIZE (SP_REGNUM), sp + frame_size);
    }
  else
    {
      /* Get the register at the offset from the context.
         XXX - if it returns a non-zero value, we should fail! */
      i = regmap[regno];
      if (i == NOT_IN_CONTEXT)
	return;			/* we can't fetch it */
      regaddr = sp + i * REGISTER_RAW_SIZE (regno);
      target_read_memory (regaddr, buf, REGISTER_RAW_SIZE (regno));
    }

  supply_register (regno, buf);
}

/* Store registers for "sk" process whose PID is "inferior_ptid".
   Must not be called if "inferior_ptid" equals "sk_running_proc",
   as the registers for the running process are not where they'd
   be if the thread weren't running. */
void
netapp_store_thread_registers (int regno, char *regs)
{
  CORE_ADDR sp;

  init_from_sk_hw_switch_proc_desc ();

  /* Get the SP for the process. */
  sp = netapp_get_proc_sp (PIDGET (inferior_ptid)); 

  if (regno >= 0)
    {
      /* Set the register in question. */
      netapp_store_thread_register (sp, regno, regs);
    }
  else
    {
      /* Set all the registers. */
      for (regno = 0; regno < NUM_REGS; regno++)
	netapp_store_thread_register (sp, regno, regs);
    }
}

static void
netapp_store_thread_register (CORE_ADDR sp, int regno, char *regs)
{
  int i;
  CORE_ADDR regaddr;
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];

  if (regno == SP_REGNUM)
    {
      /* Sorry, you can't set this. */
    }
  else
    {
      /* Get the register at the offset from the context.
         XXX - if it returns a non-zero value, we should fail! */
      i = regmap[regno];
      if (i == NOT_IN_CONTEXT)
	return;			/* we can't set it */
      regaddr = sp + i * REGISTER_RAW_SIZE (regno);
      target_write_memory (regaddr,
			   &regs[REGISTER_BYTE (regno)], 
			   REGISTER_RAW_SIZE (regno));
    }
}

/* Find the procedure descriptor for "sk_hw_switch()", and, if we find it,
   use the frame size and register mask from it. */
static void
init_from_sk_hw_switch_proc_desc (void)
{
  static int run_once;
  struct minimal_symbol *sk_hw_switch_symp;
  CORE_ADDR sk_hw_switch_addr;
  struct block *b;
  struct symbol *sym;
  CORE_ADDR startaddr;
  alpha_extra_func_info_t proc_desc;
  unsigned long mask;
  unsigned long fregmask;
  int ireg;
  int reg_index;

  if (run_once)
    return;

  /* Sigh.  This duplicates some of what "find_proc_desc()" does, because
     we can't use "find_proc_desc()", because it fetches registers
     and the attempt to fetch registers will cause us to be called.... */
  sk_hw_switch_symp = lookup_minimal_symbol ("sk_hw_switch", NULL,
					     (struct objfile *) NULL);
  if (sk_hw_switch_symp == NULL)
    sym = NULL;
  else
    {
      sk_hw_switch_addr = SYMBOL_VALUE_ADDRESS (sk_hw_switch_symp);
      b = block_for_pc (sk_hw_switch_addr);

      find_pc_partial_function (sk_hw_switch_addr, NULL, &startaddr, NULL);
      if (b == NULL)
	sym = NULL;
      else
	{
	  if (startaddr > BLOCK_START (b))
	    /* This is the "pathological" case referred to in a comment in
	       print_frame_info.  It might be better to move this check into
	       symbol reading.  */
	    sym = NULL;
	  else
	    sym = lookup_symbol (MIPS_EFI_SYMBOL_NAME, b, LABEL_NAMESPACE,
				 0, NULL);
	}
    }

    if (sym)
      {
	proc_desc = (alpha_extra_func_info_t) SYMBOL_VALUE (sym);
	/* If we never found a PDR for this function in symbol reading, then
	   give up. */
	if (proc_desc->pdr.framereg == -1)
	  proc_desc = NULL;
	else
	  {
	    /* If function is frameless, then we probably don't have a real
	       procedure descriptor.  */
	    if (proc_desc->pdr.framereg == SP_REGNUM
		&& proc_desc->pdr.frameoffset == 0)
	      proc_desc = NULL;
	  }
      }
    else
      proc_desc = NULL;

  if (proc_desc == NULL)
    {
      /* No procedure descriptor (or we couldn't find "sk_hw_switch"), so we
         assume the frame is 64 bytes and that we save S0 through S6. */
      mask = 
	(1 << 9)  |
	(1 << 10) |
	(1 << 11) |
	(1 << 12) |
	(1 << 13) |
	(1 << 14) |
	(1 << 15);
      fregmask = 0;
      frame_size = 64;
    }
  else
    {
      /* Get the frame size and register save mask from the procedure
         descriptor. */
      mask = proc_desc->pdr.regmask;
      fregmask = proc_desc->pdr.fregmask;
      frame_size = proc_desc->pdr.frameoffset;
    }

    /* Now compute the offsets in "regmap[]". */
    for (ireg = 0; ireg < NUM_REGS ; ++ireg)
      regmap[ireg] = NOT_IN_CONTEXT;
    reg_index = 1;	/* offset 0 is our RA, caller's PC */
    for (ireg = 0; ireg <= 31 ; ++ireg)
      {
	if (mask & (1 << ireg))
	  {
	    regmap[ireg] = reg_index;
	    reg_index++;
	  }
      }

    for (ireg = 0; ireg <= 31 ; ++ireg)
      {
	if (fregmask & (1 << ireg))
	  {
	    regmap[FP0_REGNUM+ireg] = reg_index;
	    reg_index++;
	  }
      }

  regmap[PC_REGNUM] = 0;

  run_once = 1;
}

#if 0
/* Get the value of the "call_firmware_ebp" variable. */
CORE_ADDR
netapp_get_call_firmware_ebp (void)
{
  struct minimal_symbol *call_firmware_ebp_symp;
  static CORE_ADDR call_firmware_ebp_addr;
  char buf[TARGET_PTR_BIT / TARGET_CHAR_BIT];
  CORE_ADDR call_firmware_ebp;

  /* Get the address of "call_firmware_ebp" if we don't already
     have it. */
  if (call_firmware_ebp_addr == 0)
    {
      call_firmware_ebp_symp = lookup_minimal_symbol ("call_firmware_ebp", 
						      NULL,
						      (struct objfile *) NULL);
      if (call_firmware_ebp_symp == NULL)
	return 0;		/* not defined? */

      call_firmware_ebp_addr = SYMBOL_VALUE_ADDRESS (call_firmware_ebp_symp);
    }

  /* Get the value of "call_firmware_ebp".
     XXX - if it returns a non-zero value, we should fail! */
  target_read_memory (call_firmware_ebp_addr, buf,
		      TARGET_PTR_BIT / TARGET_CHAR_BIT);
  call_firmware_ebp = extract_address (buf, TARGET_PTR_BIT / TARGET_CHAR_BIT);

  return call_firmware_ebp;
}
#endif
