/*
 * gdb-cpu.c -- CPU specific support for GDB stub.
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

#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "gdb.h"

/*
 *  Get byte offset from start of a saved regs struct
 *  to a given register. Register numbers are defined
 *  by gdb.
 */
int
bsp_regbyte(int regno)
{
    if (regno < 32)
	return (int)&((ex_regs_t *)0)->_gpr[regno];
    
    if (regno < 64)
	return (int)&((ex_regs_t *)0)->_fpr[regno - 32];

#define PC_REGNUM  64	/* Program counter (instruction address %iar) */
#define PS_REGNUM  65	/* Processor (or machine) status (%msr) */
#define	CR_REGNUM  66	/* Condition register */
#define	LR_REGNUM  67	/* Link register */
#define	CTR_REGNUM 68	/* Count register */
#define	XER_REGNUM 69	/* Fixed point exception registers */
#define	MQ_REGNUM  70	/* Multiply/quotient register */

    switch (regno) {
      case PC_REGNUM:
	return (int)&((ex_regs_t *)0)->_nip;
      case PS_REGNUM:
	return (int)&((ex_regs_t *)0)->_msr;
      case CR_REGNUM:
	return (int)&((ex_regs_t *)0)->_ccr;
      case LR_REGNUM:
	return (int)&((ex_regs_t *)0)->_lr;
      case CTR_REGNUM:
	return (int)&((ex_regs_t *)0)->_cnt;
      case XER_REGNUM:
	return (int)&((ex_regs_t *)0)->_xer;
      case MQ_REGNUM:
	return (int)&((ex_regs_t *)0)->_mq;
    }
    return 0;
}


/*
 * Return size in bytes of given register.
 *
 * Note that the unsigned cast here forces the result of the
 * subtractiion to very high positive values if N < FP0_REGNUM
 */
int
bsp_regsize(int regno)
{
#define FP0_REGNUM 32
    return ((unsigned)(regno) - FP0_REGNUM) < 32 ? 8 : 4;
}


/*
 *  Given an exception number and a pointer to saved registers,
 *  return a GDB signal value.
 */
int
bsp_get_signal(int exc_nr, void *saved_regs)
{
    ex_regs_t *regs = saved_regs;
    int sig = TARGET_SIGNAL_TRAP;

    switch (regs->_trap) {
      case BSP_EXC_MCHK:
#if defined(__CPU_PPC4XX__)
	{
	    unsigned esr_val, besr_val;

	    esr_val = __get_esr();
	    besr_val = __get_besr();

	    if (esr_val & 0xc0000000) {
		sig = TARGET_SIGNAL_SEGV;
		__set_esr(0);
	    } else if (esr_val & 0x30000000) {
		sig = TARGET_SIGNAL_BUS;
		__set_esr(0);
	    } else {
		sig = TARGET_SIGNAL_BUS;
		__set_besr(0);
	    }
	    /*
	     *  Set RI and ME bits of MSR.
	     */
	    asm volatile ( "mfmsr   3\n"         \
			   "ori     3,3,4096\n"  \
			   "mtmsr   3\n");
	    
	}
#else
	/*
	 *  Set RI and ME bits of MSR.
	 */
	asm volatile ( "mfmsr	3\n"         \
		       "ori     3,3,4098\n"  \
		       "mtmsr	3\n");
	sig = TARGET_SIGNAL_BUS;
#endif
	break;

      case BSP_EXC_EXTIRQ:
	sig = TARGET_SIGNAL_INT;
	break;

      case BSP_EXC_ALIGN:
	sig = TARGET_SIGNAL_BUS;
	break;

      case BSP_EXC_TRAP:
	break;

      case BSP_EXC_SYSCALL:
	break;

      case BSP_EXC_TRACE:
	break;

      case BSP_EXC_NOFP:
      case BSP_EXC_FPASSIST:
	sig = TARGET_SIGNAL_ILL;
	break;

      case BSP_EXC_DSI:
      case BSP_EXC_ISI:
	sig = TARGET_SIGNAL_SEGV;
	break;

#if defined(__CPU_MPC8XX__)
      case BSP_EXC_EMUL:
	bsp_printf("EMUL: insn<0x%x>\n", *(unsigned *)(regs->_nip));
	sig = TARGET_SIGNAL_ILL;
	break;

      case BSP_EXC_IMISS:
      case BSP_EXC_DMISS:
      case BSP_EXC_IERR:
      case BSP_EXC_DERR:
	sig = TARGET_SIGNAL_SEGV;
	break;
#endif
    }

    return sig;
}


/*
 * Set the PC value in the saved registers.
 */
void
bsp_set_pc(unsigned long pc, void *saved_regs)
{
    ((ex_regs_t *)saved_regs)->_nip = pc;
}


/*
 * Get the PC value from the saved registers.
 */
unsigned long
bsp_get_pc(void *saved_regs)
{
    return ((ex_regs_t *)saved_regs)->_nip;
}








