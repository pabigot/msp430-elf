/* Copyright (C) 2008, 2009 Free Software Foundation, Inc.
   Contributed by Diego Novillo <dnovillo@google.com>
   and Nick Clifton <nickc@redhat.com>.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GNU CC is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-pass.h"
#include "tree-flow.h"
#include "tree-dump.h"
#include "basic-block.h"
#include "domwalk.h"
#include "langhooks.h"
#include "pointer-set.h"
#include "params.h"


typedef struct pending_conversion
{
  tree                        var;    /* The static local variable.  */
  basic_block                 bb;     /* The block where it was initialised.  */
  struct pending_conversion * next;   /* The next variable in the list.  */
}
pending_conversion;

/* List of possible static locals that might be suitable for conversion.  */
static pending_conversion * pending_conversions = NULL;
/* List of free'd pending_conversion nodes so that we
   do not continually allocate and deallocate them.  */
static pending_conversion * free_conversions = NULL;

/* Keep track of the variables we have seen.  */
static struct pointer_set_t * visited = NULL;

/* Counters used for deciding whether to perform the optimization.  */
static unsigned int num_locals = 0;
static unsigned int num_statics = 0;


/* Record that VAR has been examined.  */

static inline void
mark_as_seen (tree var)
{
  pointer_set_insert (visited, var);
}

/* Returns true if VAR has already been examined.  */

static inline bool
already_seen (tree var)
{
  return pointer_set_contains (visited, var);
}

/* Get a free node on the pending conversions list.  */

static pending_conversion *
get_free_pending (void)
{
  pending_conversion * ret;

  if (free_conversions != NULL)
    {
      ret = free_conversions;
      free_conversions = free_conversions->next;
      ret->next = NULL; /* Paranoia.  */
      return ret;
    }

  return (struct pending_conversion *) xcalloc (1, sizeof * ret);
}

/* Add {VAR, BB} to our list of possible variables suitable for conversion
   to static locals.  It is permissible for VAR to appear more than once in
   this list (with different BBs) as the variable might be initialised in
   multiple locations.  */

static void
add_to_pending_list (tree var, basic_block bb)
{
  pending_conversion * pend = get_free_pending ();

  /* FIXME: To save memory (and maybe time) we ought to check for and
     ignore multiple definitions of the same var in the same block.  */
  pend->var = var;
  pend->bb = bb;
  pend->next = pending_conversions;
  pending_conversions = pend;
}

/* Remove PEND from the list of pending conversions.  PREV is the
   node just before PEND on the list.  */

static inline void
remove_from_pending_list (pending_conversion * pend,
			  pending_conversion * prev)
{
  if (prev && prev->next == pend)
    prev->next = pend->next;
  else
    pending_conversions = pend->next;

  pend->next = free_conversions;
  free_conversions = pend;

  pend->var = NULL; /* Paranoia.  */
  pend->bb = NULL;  /* Paranoia.  */
}

/* Scan the list of pending conversions and remove all occurrences of VAR.  */

static void
remove_all_occurrences_from_pending_list (tree var)
{
  pending_conversion * pend;
  pending_conversion * prev = NULL;

  for (pend = pending_conversions ; pend ; pend = pend->next)
    {
      if (pend->var == var)
	remove_from_pending_list (pend, prev);
      prev = pend;
    }
}

/* Lookup VAR in our list of pending conversions.  If we find a match check
   to see that the BBs of ALL of the sites of the initialisation of VAR
   dominate the BB we have been given (the site of a reference).  If this
   condition does not hold then ALL occurrences of the variable must be
   removed from the list.

   Return false if either VAR was not found on the list or it was found but
   it was removed from the list.  In such cases the use of VAR has determined
   that the variable is unsuitable for conversion and so it should be marked
   as such.  */

static inline bool
scan_pending_list (tree var, basic_block bb)
{
  bool ret = false;
  pending_conversion * pend;

  for (pend = pending_conversions ; pend ; pend = pend->next)
    if (pend->var == var)
      {
	if (bb != pend->bb)
	  {
	    if (!dominated_by_p (CDI_DOMINATORS, bb, pend->bb))
	      {
		basic_block pends_bb = pend->bb;

		remove_all_occurrences_from_pending_list (var);

		if (dump_file && (dump_flags & TDF_DETAILS))
		  fprintf (dump_file, "'%s' not converted because DEF in"
			   "block %d did not dominate USE in block %d.\n",
			   lang_hooks.decl_printable_name (var, 0),
			   pends_bb->index,
			   bb->index);

		return false;
	      }

	    if (dump_file && (dump_flags & TDF_DETAILS))
	      fprintf (dump_file, "DEF of '%s' in block %d "
		       "dominates USE in block %d.\n",
		       lang_hooks.decl_printable_name (var, 0),
		       pend->bb->index, bb->index);
	  }

	ret = true;
      }

  if (ret == false
      && pending_conversions != NULL
      && dump_file
      && (dump_flags & TDF_DETAILS))
    fprintf (dump_file, "'%s' USEd in block %d and not on pending list, so"
	     " it cannot be made auto.\n",
	     lang_hooks.decl_printable_name (var, 0),
	     bb->index);

  return ret;
}

/* Convert VAR from a local static to a local auto.  */

static void
make_auto (tree var)
{
  if (already_seen (var))
    return;

  mark_as_seen (var);

  TREE_STATIC (var) = 0;
  /* Since the var is no longer static, it no
     longer needs space in the assembler file.  */
  TREE_ASM_WRITTEN (var) = 1;

  if (dump_file)
    fprintf (dump_file, "Static local '%s' has been converted to auto.\n",
	     lang_hooks.decl_printable_name (var, 0));
}

/* Record that VAR has been examined and that we have decided
   that it is not suitable for conversion into a auto.  */

static void
do_not_make_auto (tree var)
{
  if (already_seen (var))
    return;

  mark_as_seen (var);

  if (dump_file && (dump_flags & TDF_DETAILS))
    fprintf (dump_file, "Static local '%s' is not suitable for conversion"
	     " to auto.\n",
	     lang_hooks.decl_printable_name (var, 0));
}

/* Returns true if DECL is a local static suitable
   for consideration by this optimization.  */

static inline bool
is_local_static_var (tree decl)
{
  return
    decl != NULL
    /* Check that we are looking at a variable declaration.  */
    && TREE_CODE (decl) == VAR_DECL
    /* That has storage allocated in this
       compilation unit (rather than on the stack).  */
    && TREE_STATIC (decl)
    /* Which is not visible externally to this compilation unit.  */
    && ! TREE_PUBLIC (decl)
    /* Which is allocated inside a function.  */
    && decl_function_context (decl) != NULL
    /* Which was not created by the compiler.  */
    && ! DECL_ARTIFICIAL (decl)
    /* Which is not used in a nested function.  */
    && ! DECL_NONLOCAL (decl)
    /* Which does not have its address taken.  */
    && ! TREE_ADDRESSABLE (decl);
}

/* Returns true if DECL is a non-static local variable.  */

static inline bool
is_local_auto_var (tree decl)
{
  return
    /* Check that we are looking at a variable declaration.  */
    TREE_CODE (decl) == VAR_DECL
    /* That does not have storage allocated in this compilation unit.  */
    && ! TREE_STATIC (decl)
    /* Which is not visible externally to this compilation unit.  */
    && ! TREE_PUBLIC (decl)
    /* Which is allocated inside a function.  */
    && decl_function_context (decl) != NULL
    /* And which has a programmer assigned name.  */
    && DECL_NAME (decl) != NULL;
}

/* Transform static declarations into auto declarations.  This allows the
   variable to be allocated to a register.  A static variable V can be
   converted iff:

   1- V is entirely local to the function.

   2- There exists a definition for V that post-dominates the entry block
      (V's initialization is not considered a definition).

   Condition 2 stops certain cases where the optimization could take
   place.  Consider:

     int foo (void)
     {
       static int could_be_made_auto = 0;

       if (external_func ())                   // #1
         return 1;

       return could_be_made_auto = 3;          // #2
     }

   In this case the assignment at #2 does not post dominate the entry block
   since there is a path via entry->#1->exit, but nevertheless the
   optimization would be valid.

   Thus when we encounter an initialization of V (when we have not previously
   encountered a use of V) we check to see if it post-dominates the entry
   block.  If it does the optimization can be performed, but if it does not,
   then it is saved onto a list.  Meanwhile we continue checking the other
   blocks.  If we later encounter a block that uses V and which is not
   dominated by our saved block then we must delete our saved block.  At the
   end of the walk any saved blocks still remaining on our list can also be
   converted.

   There is another opportunity that might be missed as well:

     extern int a,b;
     
     int foo (void)
     {
       static int could_be_made_auto = 0;

       if (external_func ())                 // #1
         {
	   a = could_be_made_auto;           // #2
	   could_be_made_auto = 2;           // #3
         }
       b = could_be_made_auto;               // #4
     }

   The problem here is the second DEF at #3.  It does not post-dominate the
   entry block (because of #1) and it does not dominate the USE at #3.
   We catch this case by looking for DEFs that are dominated by another,
   already seen and pending DEF.  In this case we can ignore the dominated
   DEF.  */

/* Looks at the right hand side (RHS) and left hand side (LHS) of a modify
   expression in basic block BB and decides if they are relevant to this
   optimization.  */

static void
process (tree rhs, tree lhs, basic_block bb)
{
  /* See if we have a static local on the right hand side of the expression.
     If we do look it up in the pending list.  If it is not there (or it was
     there but the scan removed it) then mark it as being unsuitable for
     conversion.  If it is (still) present on the list then do not mark it as
     unsuitable since we know that this USE is dominated by all of the
     currently located definitions.  It does not matter if this USE dominates
     an as yet undiscovered definition since that definition must also be
     dominated by the nodes on the pending list that are dominating this USE.  */
  if (is_local_static_var (rhs)
      && ! scan_pending_list (rhs, bb))
    do_not_make_auto (rhs);

  /* See if we have a static local on the left hand side of the expression,
     which post-dominates the entry block.  Note - this test is performed
     after the test of the rhs in case the same variable is referenced on
     both sides of the expression.  */
  if (is_local_static_var (lhs))
    {
      edge_iterator ei;
      edge e;

      /* If we have already decided the fate of this variable
	 then we do not need to do anything else here.  */
      if (already_seen (lhs))
	return;

      /* Because gcc supports multiple entry points to a function the
	 dominator tree does not start with ENTRY_BLOCK_PTR but instead with
	 its successors, so we must check all of them.  */
      FOR_EACH_EDGE (e, ei, ENTRY_BLOCK_PTR->succs)
	if (! dominated_by_p (CDI_POST_DOMINATORS, e->dest, bb))
	  break;

      if (e == NULL)
	{
	  /* If this variable was on our list of pending conversions we can
	     now remove it as we have found proof that it is safe to convert.  */
	  remove_all_occurrences_from_pending_list (lhs);

	  make_auto (lhs);
	}
      else
	{
	  pending_conversion * pend;

	  for (pend = pending_conversions ; pend ; pend = pend->next)
	    if (pend->var == lhs
		&& bb != pend->bb
		&& dominated_by_p (CDI_DOMINATORS, bb, pend->bb))
	      {
		if (dump_file && (dump_flags & TDF_DETAILS))
		  fprintf (dump_file, "Ignoring DEF of '%s' in block %d "
			   "because it is dominated by an already seen "
			   "DEF in block %d.\n",
			   lang_hooks.decl_printable_name (lhs, 0),
			   bb->index, pend->bb->index);
		return;
	      }
	  
	  add_to_pending_list (lhs, bb);

	  if (dump_file && (dump_flags & TDF_DETAILS))
	    fprintf (dump_file, "Add DEF of '%s' in block %d to pending list"
		     " because it does not post-dominate entry block %d.\n",
		     lang_hooks.decl_printable_name (lhs, 0),
		     bb->index, e->dest->index);
	}
    }
}

/* Utility function called from walk_dominator_tree that examines statement
   SI in basic block BB and runs the optimization for modify expressions.  */

static void
locate_and_convert_statics (struct dom_walk_data * walk_data ATTRIBUTE_UNUSED,
			    basic_block bb,
			    block_stmt_iterator si)
{
  tree stmt = bsi_stmt (si);

  if (TREE_CODE (stmt) == MODIFY_EXPR)
    process (TREE_OPERAND (stmt, 1), TREE_OPERAND (stmt, 0), bb);

  else if (TREE_CODE (stmt) == GIMPLE_MODIFY_STMT)
    process (GIMPLE_STMT_OPERAND (stmt, 1), GIMPLE_STMT_OPERAND (stmt, 0), bb);
}

/* Count the number of USEd locals (auto and static).
   We are only interested in USEd locals as these are the
   ones that are going to occupy space in the stack frame.  */

static void
count_locals_and_statics (struct dom_walk_data * walk_data ATTRIBUTE_UNUSED,
			  basic_block bb ATTRIBUTE_UNUSED,
			  block_stmt_iterator si)
{
  tree stmt, var;

  stmt = bsi_stmt (si);

  if (TREE_CODE (stmt) != MODIFY_EXPR)
    return;

  var = TREE_OPERAND (stmt, 1);

  if (is_local_static_var (var))
    {
      if (! already_seen (var))
	{
	  ++ num_statics;
	  mark_as_seen (var);
	}
    }
  else if (is_local_auto_var (var))
    {
      if (! already_seen (var))
	{
	  ++ num_locals;
	  mark_as_seen (var);
	}
    }
}

/* Run the local statics optimization.  */

static unsigned int
tree_optimize_local_statics (void)
{
  unsigned int locals_threshold;
  struct dom_walk_data walk_data;
  bool doit;

  locals_threshold = PARAM_VALUE (PARAM_SLO_LOCAL_VARIABLES_THRESHOLD);

  /* We're going to do a dominator walk, so
     ensure that we have dominance information.  */
  calculate_dominance_info (CDI_DOMINATORS);
  calculate_dominance_info (CDI_POST_DOMINATORS);

  /* Setup callbacks for the generic dominator tree walker.  */
  walk_data.walk_stmts_backward = false;
  walk_data.dom_direction = CDI_DOMINATORS;
  walk_data.initialize_block_local_data = NULL;
  walk_data.before_dom_children_before_stmts = NULL;
  walk_data.before_dom_children_after_stmts = NULL;
  walk_data.after_dom_children_before_stmts = NULL;
  walk_data.after_dom_children_walk_stmts = NULL;
  walk_data.after_dom_children_after_stmts = NULL;
  walk_data.global_data = NULL;
  walk_data.block_local_data_size = 0;
  walk_data.interesting_blocks = NULL;

  doit = true;

  if (locals_threshold)
    {
      num_locals = 0;
      num_statics = 0;

      visited = pointer_set_create ();

      walk_data.before_dom_children_walk_stmts = count_locals_and_statics;

      init_walk_dominator_tree (& walk_data);
      walk_dominator_tree (& walk_data, ENTRY_BLOCK_PTR);
      fini_walk_dominator_tree (& walk_data);
      
      if (num_locals > locals_threshold || num_statics == 0)
	{
	  doit = false;

	  if (num_statics > 0
	      && dump_file
	      && (dump_flags & TDF_DETAILS))
	    fprintf (dump_file, "Statics not converted as there are too many"
		     " active locals.\n");
	}

      pointer_set_destroy (visited);
    }

  if (doit)
    {
      pending_conversion * pend;

      visited = pointer_set_create ();

      walk_data.before_dom_children_walk_stmts = locate_and_convert_statics;

      init_walk_dominator_tree (& walk_data);
      walk_dominator_tree (& walk_data, ENTRY_BLOCK_PTR);
      fini_walk_dominator_tree (& walk_data);

      /* Walk the pending list converting the remaining statics into autos.  */
      while ((pend = pending_conversions) != NULL)
	{
	  if (! already_seen (pend->var))
	    {
	      make_auto (pend->var);

	      if (dump_file
		  && (dump_flags & TDF_DETAILS))
		fprintf (dump_file, "Static local '%s' converted because "
			 "no un-dominated USEs were found.\n",
			 lang_hooks.decl_printable_name (pend->var, 0));
	    }

	  remove_from_pending_list (pend, NULL);
	}

      pointer_set_destroy (visited);
      visited = NULL;
    }

  free_dominance_info (CDI_DOMINATORS);
  free_dominance_info (CDI_POST_DOMINATORS);

  return 0;
}

static bool
gate_optimize_local_statics (void)
{
  return flag_optimize_local_statics;
}

struct gimple_opt_pass pass_optimize_local_statics =
{
  {
    GIMPLE_PASS,
    "optimize_local_statics",	/* Name.  */
    gate_optimize_local_statics,/* Gate.  */
    tree_optimize_local_statics,/* Execute.  */
    NULL,			/* Sub.  */
    NULL,			/* Next.  */
    0,			 	/* Static_pass_number.  */
    0,			 	/* Tv_id.  */
    PROP_cfg,		 	/* Properties_required.  */
    0,			 	/* Properties_provided.  */
    0,			 	/* Properties_destroyed.  */
    0,			 	/* Todo_flags_start.  */
    TODO_dump_cgraph | TODO_verify_rtl_sharing /* Todo_flags_finish.  */
  }
};
