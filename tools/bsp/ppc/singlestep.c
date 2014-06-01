/*
 * singlestep.c -- PPC specific single-step support.
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

#define STEP_WITH_BREAKPOINTS 1

#if defined(STEP_WITH_BREAKPOINTS)

struct xl_form {
    unsigned int op : 6;
    unsigned int bo : 5;
    unsigned int bi : 5;
    unsigned int reserved : 5;
    unsigned int xo : 10;
    unsigned int lk : 1;
};

struct i_form {
    unsigned int op : 6;
    signed   int li : 24;
    unsigned int aa : 1;
    unsigned int lk : 1;
};

struct b_form {
    unsigned int op : 6;
    unsigned int bo : 5;
    unsigned int bi : 5;
    signed   int bd : 14;
    unsigned int aa : 1;
    unsigned int lk : 1;
};

union ppc_insn {
    unsigned int   word;
    struct i_form  i;
    struct b_form  b;
    struct xl_form xl;
};


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


static void
set_single_bp(int index, unsigned int *targ)
{
    step_bp[index].addr = targ;
    step_bp[index].opcode = *targ;
    *targ = BREAKPOINT_OPCODE;
    bsp_flush_dcache(targ, 4);
    bsp_flush_icache(targ, 4);
}
#endif

/*
 *  Cleanup after a trap.
 */
void
bsp_singlestep_cleanup(void *saved_regs)
{
#if defined(STEP_WITH_BREAKPOINTS)
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
#else
#if defined(__CPU_PPC4XX__)
    unsigned dbcr_val;
    unsigned dbsr_val;

    dbcr_val = __get_dbcr();
    dbsr_val = __get_dbsr();

#if 0
    bsp_printf("dbcr[0x%08x] dbsr[0x%08x] msr[0x%08x]\n",
	       dbcr_val, dbsr_val, ((ex_regs_t *)saved_regs)->_msr);
#endif

    if (dbcr_val & 0x40000000) {
	dbcr_val &= ~0x48000000;
	__set_dbcr(dbcr_val);

	__set_dbsr(0x80000000);

	((ex_regs_t *)saved_regs)->_nip += 4;
    }

    ((ex_regs_t *)saved_regs)->_msr &= ~MSR_DE;
#else
    ((ex_regs_t *)saved_regs)->_msr &= ~MSR_SE;
#endif
#endif
}


void
bsp_singlestep_setup(void *saved_regs)
{
#if defined(STEP_WITH_BREAKPOINTS)
    ex_regs_t *regs = saved_regs;
    union ppc_insn insn;
    unsigned long targ;

    targ = regs->_nip;
    insn.word = *(unsigned int *)targ;

    switch (insn.i.op) {
      case 16:
	/* bcx */
	set_single_bp(0, (unsigned int *)(targ + 4));
	if (insn.b.aa) 
	    set_single_bp(1, (unsigned int *)(insn.b.bd << 2));
	else
	    set_single_bp(1, (unsigned int *)((insn.b.bd << 2) + (int)targ));
	return;

      case 18:
	/* bx */
	if (insn.i.aa)
	    set_single_bp(0, (unsigned int *)(insn.i.li << 2));
	else
	    set_single_bp(0, (unsigned int *)((insn.i.li << 2) + (int)targ));
	return;

      case 19:
	if (insn.xl.reserved == 0) {
	    if (insn.xl.xo == 528) {
		/* bcctrx */
		set_single_bp(0, (unsigned int *)(targ + 4));
		set_single_bp(1, (unsigned int *)(regs->_cnt & ~3));
		return;
	    } else if (insn.xl.xo == 16) {
		/* bclrx */
		set_single_bp(0, (unsigned int *)(targ + 4));
		set_single_bp(1, (unsigned int *)(regs->_lr & ~3));
		return;
	    }
	}
	break;
      default:
	break;
    }
    set_single_bp(0, (unsigned int *)(targ + 4));
    
#else
#if defined(__CPU_PPC4XX__)
    unsigned dbcr_val, msr;

    msr = __get_msr();
    msr &= ~MSR_DE;
    __set_msr(msr);

    dbcr_val = __get_dbcr();
    dbcr_val |= 0x48000000;
    __set_dbcr(dbcr_val);

    ((ex_regs_t *)saved_regs)->_msr |= MSR_DE;
#else
    ((ex_regs_t *)saved_regs)->_msr |= MSR_SE;
#endif
#endif
}

void
bsp_skip_instruction(void *saved_regs)
{
    ((ex_regs_t *)saved_regs)->_nip += 4;
}

