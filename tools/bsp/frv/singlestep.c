/*
 * singlestep.c -- FR-V specific single-step support.
 *
 * Copyright (c) 1999, 2000 Red Hat, Inc.
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
#include "fr500.h"

#ifdef __CPU_TIGER__
#define USE_HW_STEP 1
#define TIGER_WORKAROUND 1
#endif

#if USE_HW_STEP
#if TIGER_WORKAROUND
static int step_flag;
static unsigned step_opcode;
#endif /* TIGER_WORKAROUND */
#else
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
#endif

#if USE_HW_STEP && TIGER_WORKAROUND
static int
must_emulate(ex_regs_t *regs)
{
    unsigned opcode;

    /* if interrupts are already masked, no need to workaround */
    if ((regs->_psr & PSR_PIL_MASK) == PSR_PIL_MASK)
	return 0;

    opcode = *(unsigned *)(regs->_pc);

    /* Only need to emulate insns that get or set PIL.
     * This means movgs/movsg to PSR.
     */
    if ((opcode & 0x7fffff80) == 0x000c0180)
	return 1;

    return 0;
}

static void
step_emulate(ex_regs_t *regs)
{
    unsigned opcode, grj;

    opcode = *(unsigned *)(regs->_pc);
    grj = opcode & 0x3f;

    /* do the move */
    if (opcode & 0x40) {
	/* movsg */
	regs->_gpr[grj] = regs->_psr;
    } else {
	/* movgs */
	regs->_psr = regs->_gpr[grj];
    }
    regs->_pc += 4;
    regs->_bpcsr = regs->_pc;
}
#endif /* TIGER_WORKAROUND */

/*
 *  Cleanup after a singlestep.
 */
void
bsp_singlestep_cleanup(void *saved_regs)
{
#if USE_HW_STEP
    __set_dcr(__get_dcr() & ~DCR_SE);

#if TIGER_WORKAROUND
    if (__get_brr() & BRR_ST) {
	ex_regs_t *regs = saved_regs;

	regs->_psr &= ~PSR_PIL_MASK;
	regs->_psr |= step_flag;

	if (must_emulate(regs)) {
	    *(unsigned *)(regs->_pc) = step_opcode;
	    bsp_flush_dcache((void *)regs->_pc, 4);
	    bsp_flush_icache((void *)regs->_pc, 4);
	    step_emulate(saved_regs);
	}
    }
#endif /* TIGER_WORKAROUND */
#else
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
#endif
}

/*
 * Set breakpoint instructions for single stepping.
 */
void
bsp_singlestep_setup(void *saved_regs)
{
    ex_regs_t *regs = saved_regs;
#if USE_HW_STEP
    __set_dcr(__get_dcr() | DCR_SE);
#if TIGER_WORKAROUND
    /* don't let hw step get interrupted */
    step_flag = regs->_psr & PSR_PIL_MASK;
    regs->_psr |= PSR_PIL_MASK;

    if (must_emulate(regs)) {
	/* we replace it with a breakpoint, mask interrupts and save flags */
	step_opcode = *(unsigned *)(regs->_pc);
	*(unsigned *)(regs->_pc) = BREAKPOINT_OPCODE;
	bsp_flush_dcache((void *)regs->_pc, 4);
	bsp_flush_icache((void *)regs->_pc, 4);
    }
#endif /* TIGER_WORKAROUND */
#else
    unsigned opcode;
    unsigned pc, targ;
    int n, is_branch = 0;

    pc = targ = regs->_pc;
    opcode = *(unsigned *)pc;

    switch ((opcode >> 18) & 0x7f) {
      case 6:
      case 7:
	/* bcc, fbcc */
	is_branch = 1;
	n = (int)(opcode << 16);
	n >>= 16;
	targ = pc + n*4;
	pc += 4;
	break;
      case 12:
	/* jmpl */
	n = (int)(regs->_gpr[(opcode>>12)&63]);
	n += (int)(regs->_gpr[opcode&63]);
	pc = n;
	break;
      case 13:
	/* jmpil */
	n = (int)(regs->_gpr[(opcode>>12)&63]);
	n += (((int)(opcode << 20)) >> 20);
	pc = n;
	break;
      case 15:
	/* call */
	n = (opcode >> 25) << 18;
	n |= (opcode & 0x3ffff);
	n <<= 8;
	n >>= 8;
	pc += n*4;
	break;
      case 14:
	is_branch = 1;
	targ = regs->_lr;
	pc += 4;
	break;
      default:
	pc += 4;
	break;
    }
    step_bp[0].addr = (unsigned *)pc;
    step_bp[0].opcode = *(unsigned *)pc;
    *(unsigned *)pc = BREAKPOINT_OPCODE;
    bsp_flush_dcache((void *)pc, 4);
    bsp_flush_icache((void *)pc, 4);

    if (is_branch && pc != targ) {
	step_bp[1].addr = (unsigned*)targ;
	step_bp[1].opcode = *(unsigned *)targ;
	*(unsigned *)targ = BREAKPOINT_OPCODE;
	bsp_flush_dcache((void *)targ, 4);
	bsp_flush_icache((void *)targ, 4);
    } else
	step_bp[1].addr = NULL;
#endif
}


void
bsp_skip_instruction(void *saved_regs)
{
    unsigned long pc;

    pc = bsp_get_pc(saved_regs);
    pc += 4;
    bsp_set_pc(pc, saved_regs);
}
