/*
 * singlestep.c -- SPARC-specific single-step support.
 *
 * Copyright (c) 1999 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include <stdlib.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "insn.h"

/*
 * Structure to hold opcodes hoisted when breakpoints are
 * set for single-stepping or async interruption.
 */
struct _bp_save {
    unsigned int  *addr;
    unsigned int   opcode;
};

/*
 * We single-step by setting breakpoints.
 *
 * This is where we save the original instructions.
 */
static struct _bp_save step_bp[2];


/*
 *  Cleanup after a singlestep.
 */
void
bsp_singlestep_cleanup(void *saved_regs)
{
    struct _bp_save *p;

    p = &step_bp[0];
    if (p->addr) {
	*(p->addr) = p->opcode;
	bsp_flush_dcache(p->addr, 4);
	bsp_flush_icache(p->addr, 4);
	p->addr = NULL;

	p = &step_bp[1];
	if (p->addr) {
	    *(p->addr) = p->opcode;
	    bsp_flush_dcache(p->addr, 4);
	    bsp_flush_icache(p->addr, 4);
	    p->addr = NULL;
	}
    }
}


/* Check instruction at ADDR to see if it is a branch.
   All non-annulled instructions will go to NPC or will trap.
   Set *TARGET if we find a candidate branch; set to zero if not. */
branch_type
isbranch (int instruction, unsigned long addr, unsigned int *target)
{
    branch_type val = not_branch;
    int offset = 0;		/* Must be signed for sign-extend.  */

    *target = 0;

    if (X_OP (instruction) == 0
	&& (X_OP2 (instruction) == 2
	    || X_OP2 (instruction) == 6
	    || X_OP2 (instruction) == 1
	    || X_OP2 (instruction) == 3
	    || X_OP2 (instruction) == 5
#if defined(SPARC_V9)
	    || X_OP2 (instruction) == 7
#endif
	    )) {
	if (X_COND (instruction) == 8)
	    val = X_A (instruction) ? baa : ba;
	else
	    val = X_A (instruction) ? bicca : bicc;
	switch (X_OP2 (instruction)) {
	  case 2:
	  case 6:
#if defined(SPARC_V9)
	  case 7:
#endif
	    offset = 4 * X_DISP22(instruction);
	    break;
	  case 1:
	  case 5:
	    offset = 4 * X_DISP19(instruction);
	    break;
	  case 3:
	    offset = 4 * X_DISP16(instruction);
	    break;
	}
	*target = addr + offset;
    }
#if defined(SPARC_V9)
    else if (X_OP (instruction) == 2
	     && X_OP3 (instruction) == 62) {
	if (X_FCN (instruction) == 0) {
	    /* done */
	    *target = get_register (NPC);
	    val = done_retry;
	} else if (X_FCN (instruction) == 1) {
	    /* retry */
	    *target = get_register (PC);
	    val = done_retry;
	}
    }
#endif
    return val;
}


static void
set_single_bp(int index, unsigned int *targ)
{
    step_bp[index].addr = targ;
    step_bp[index].opcode = *targ;
    *targ = BREAKPOINT_OPCODE;
    bsp_flush_dcache(targ, 4);
    bsp_flush_icache(targ, 4);
}


/*
 * Set breakpoint instructions for single stepping.
 */
void
bsp_singlestep_setup(void *saved_regs)
{
    ex_regs_t     *regs = saved_regs;
    unsigned long pc = regs->_pc;
    unsigned long next_pc = regs->_npc;
    unsigned long target;
    branch_type   br;
    int           pc_insn;

    pc_insn = *(int *)pc;

    br = isbranch (pc_insn, pc, (unsigned int *)&target);

    set_single_bp(0, (unsigned int *)next_pc);

    /* Conditional annulled branch will either end up at
       npc (if taken) or at npc+4 (if not taken).
       Trap npc+4.  */
    if (br == bicca)
	set_single_bp(1, (unsigned int *)(next_pc + 4));
    /* Unconditional annulled branch will always end up at
       the target.  */
    else if (br == baa && target != next_pc)
	set_single_bp (1, (unsigned int *)target);
    else if (br == done_retry)
	set_single_bp (1, (unsigned int *)target);
}


void
bsp_skip_instruction(void *saved_regs)
{
    bsp_set_pc(((ex_regs_t *)saved_regs)->_npc, saved_regs);
}
