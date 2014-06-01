/*
 * singlestep.c -- FR30 specific single-step support.
 *
 * Copyright (c) 1998, 1999 Cygnus Support
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

#define DEBUG_SINGLESTEP 0

typedef union {
    unsigned short word;
    struct {
	unsigned short op:8;
	unsigned short rj:4;
	unsigned short ri:4;
    } a;

    struct {
	unsigned short op:4;
	unsigned short offset:8;
	unsigned short ri:4;
    } b;

    struct {
	unsigned short op:8;
	unsigned short immed:4;
	unsigned short ri:4;
    } c;

    struct {
	unsigned short op:8;
	signed short immed:8;
    } d;

    struct {
	unsigned short op:8;
	unsigned short subop:4;
	unsigned short ri:4;
    } e;

    struct {
	unsigned short op:5;
	signed short offset:11;
    } f;
} insn_t;



#if 0
/*
 *  Cleanup after a trap.
 */
void
bsp_singlestep_cleanup(void *saved_regs)
{
    ((ex_regs_t *)saved_regs)->_ps &= ~0x100;
}


void
bsp_singlestep_setup(void *saved_regs)
{
    ((ex_regs_t *)saved_regs)->_ps |= 0x100;
}
#else


/*
 * Structure to hold opcodes hoisted when breakpoints are
 * set for single-stepping or async interruption.
 */
struct _bp_save {
    unsigned short  *addr;
    unsigned short   opcode;
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
	bsp_flush_icache(p->addr, 2);
	p->addr = NULL;

	p = &step_bp[1];
	if (p->addr) {
	    *(p->addr) = p->opcode;
	    bsp_flush_icache(p->addr, 2);
	    p->addr = NULL;
	}
    }
}


static unsigned long
get_regi(int regno, ex_regs_t *regs)
{
    switch (regno) {
      case 0:	return regs->_r0;
      case 1:	return regs->_r1;
      case 2:	return regs->_r2;
      case 3:	return regs->_r3;
      case 4:	return regs->_r4;
      case 5:	return regs->_r5;
      case 6:	return regs->_r6;
      case 7:	return regs->_r7;
      case 8:	return regs->_r8;
      case 9:	return regs->_r9;
      case 10:	return regs->_r10;
      case 11:	return regs->_r11;
      case 12:	return regs->_r12;
      case 13:	return regs->_r13;
      case 14:	return regs->_r14;
      case 15:	return regs->_r15;
    }
    return 0;
}


void
bsp_singlestep_setup(void *saved_regs)
{
    ex_regs_t *regs = saved_regs;
    insn_t    insn;
    unsigned long targ, targ2;
    int is_cond;

    targ = regs->_pc;
    insn.word = *(unsigned short *)targ;
    targ += 2;
    is_cond = 0;

#if DEBUG_SINGLESTEP
    bsp_printf("Setup singlestep [0x%08x] [0x%04x]\n", regs->_pc, insn.word);
#endif

    switch (insn.b.op) {
      case 0x9:
	switch (insn.a.op) {
	  case 0x9b:
	    /* ldi:20 */
	    targ += 2;
	    break;
	  case 0x97:
	    /* type-E */
	    switch (insn.e.subop) {
	      case 0:
	      case 1:
		/* call @ri, jmp @ri */
		targ = get_regi(insn.e.ri, regs);
		break;
	      case 2:
		/* ret */
		targ = regs->_rp;
		break;
	      case 3:
		/* reti */
		targ = *((unsigned long *)(regs->_ssp - 8));
		break;
	    }
	    break;

	  case 0x9f:
	    /* type-E */
	    switch (insn.e.subop) {
	      case 0:
	      case 1:
		/* call:d @ri, jmp:d @ri */
		targ = get_regi(insn.e.ri, regs);
		break;
	      case 2:
		/* ret:d */
		targ = regs->_rp;
		break;
	      case 8:
		/* ldi:32 */
		targ += 4;
		break;
	      case 12 ... 15:
		targ += 2;
		break;
	    }
	    break;
	}
	break;
      case 0xd:
	/* call label12 */
	targ += (insn.f.offset * 2);
	break;
      case 0xe:
	/* conditional branch with no delay slot */
	if (insn.d.immed != 0) {
	    is_cond = 1;
	    targ2 = targ + (insn.d.immed * 2);
	}
	break;
      case 0xf:
	/* conditional branch with delay slot */
	targ2 = targ + (insn.d.immed * 2);
	targ += 2;  /* skip past delay slot */
	is_cond = (targ != targ2);
	break;
    }
    step_bp[0].addr = (unsigned short *)targ;
    step_bp[0].opcode = *(unsigned short *)targ;
    *(unsigned short *)targ = BREAKPOINT_OPCODE;
    bsp_flush_icache((void *)targ, 2);

#if DEBUG_SINGLESTEP
    bsp_printf("Setting BP at [0x%08x]\n", targ);
#endif

    if (is_cond) {
	step_bp[1].addr = (unsigned short *)targ2;
	step_bp[1].opcode = *(unsigned short *)targ2;
	*(unsigned short *)targ2 = BREAKPOINT_OPCODE;
	bsp_flush_icache((void *)targ2, 2);

#if DEBUG_SINGLESTEP
	bsp_printf("Setting BP at [0x%08x]\n", targ2);
#endif

    }
    else
	step_bp[1].addr = 0;
}
#endif


void
bsp_skip_instruction(void *saved_regs)
{
    insn_t    insn;

    insn.word = *(unsigned short *)(((ex_regs_t *)saved_regs)->_pc);

    switch (insn.a.op) {
      case 0x9F:
	/* Type-E */
	switch (insn.e.subop) {
	  case 0x08:
	    ((ex_regs_t *)saved_regs)->_pc += 6;
	    break;
	  case 0x0c:
	  case 0x0d:
	  case 0x0e:
	  case 0x0f:
	    ((ex_regs_t *)saved_regs)->_pc += 4;
	    break;
	}
	break;
      case 0x9b:
	((ex_regs_t *)saved_regs)->_pc += 4;
	break;
      default:
	((ex_regs_t *)saved_regs)->_pc += 2;
	break;
    }
}


