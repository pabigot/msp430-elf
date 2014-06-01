/*
 * singlestep.c -- MIPS specific single-step support.
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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


/*
 * Set breakpoint instructions for single stepping.
 */
void
bsp_singlestep_setup(void *saved_regs)
{
    ex_regs_t *regs = saved_regs;
    union mips_insn insn;
    unsigned long targ;
    int is_branch, is_cond, i;
    unsigned *addr;

    targ = regs->_pc;
    insn.word = *(unsigned int *)targ;
    is_branch = is_cond = 0;

    switch (insn.i.op) {
      case OP_SPECIAL:
	switch (insn.r.func) {
	  case FUNC_JALR:
	  case FUNC_JR:
	    targ = *(&regs->_zero + insn.r.rs);
	    is_branch = 1;
	    break;
	}
	break;

      case OP_REGIMM:
	switch (insn.i.rt) {
	  case RT_BLTZ:
	  case RT_BGEZ:
	  case RT_BLTZL:
	  case RT_BGEZL:
	  case RT_BLTZAL:
	  case RT_BGEZAL:
	  case RT_BLTZALL:
	  case RT_BGEZALL:
	    is_branch = is_cond = 1;
	    targ += 4 + (insn.i.imm << 2);
	    break;
	}
	break;

      case OP_J:
      case OP_JAL:
	is_branch = 1;
	targ += 4;
	targ >>= 28;
	targ <<= 28;
	targ |= (insn.j.target << 2);
	break;

      case OP_BEQ:
      case OP_BNE:
      case OP_BLEZ:
      case OP_BGTZ:
      case OP_BEQL:
      case OP_BNEL:
      case OP_BLEZL:
      case OP_BGTZL:
	is_branch = is_cond = 1;
	targ += 4 + (insn.i.imm << 2);
	break;

      case OP_COP0:
      case OP_COP1:
      case OP_COP2:
	switch (insn.i.rs) {
	  case COP_RS_BC:
	    is_branch = is_cond = 1;
	    targ += 4 + (insn.i.imm << 2);
	    break;
	}
	break;
    }
				
    if (is_branch) {
	i = 0;
	addr = (unsigned *)(regs->_pc + 8);
	if (is_cond && addr != (unsigned *)targ) {
	    step_bp[i].addr = addr;
	    step_bp[i++].opcode = *addr;
	    *addr = BREAKPOINT_OPCODE;
	    bsp_flush_dcache(addr, 4);
	    bsp_flush_icache(addr, 4);
	}
	step_bp[i].addr = (unsigned int *)targ;
	step_bp[i].opcode = *(unsigned *)targ;
	*(unsigned *)targ = BREAKPOINT_OPCODE;
	bsp_flush_dcache((void *)targ, 4);
	bsp_flush_icache((void *)targ, 4);
    } else {
	addr = (unsigned *)(regs->_pc + 4);
	step_bp[0].addr = addr;
	step_bp[0].opcode = *addr;
	*addr = BREAKPOINT_OPCODE;
	bsp_flush_dcache(addr, 4);
	bsp_flush_icache(addr, 4);
    }
}


void
bsp_skip_instruction(void *saved_regs)
{
    bsp_set_pc(((ex_regs_t *)saved_regs)->_pc + 4, saved_regs);
}
