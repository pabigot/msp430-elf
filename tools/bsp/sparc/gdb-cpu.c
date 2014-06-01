/*
 * gdb-cpu.c -- SPARC-specific support for GDB stub.
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

#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "gdb.h"

/*
 * Return byte offset within the saved register area of the
 * given register.
 */
int
bsp_regbyte(int regno)
{
    return regno * sizeof(unsigned long);
}


/*
 * Return size in bytes of given register.
 */
int
bsp_regsize(int regno)
{
    return 4;
}


/*
 *  Given an exception number and a pointer to saved registers,
 *  return a GDB signal value.
 */
int
bsp_get_signal(int exc_nr, void *saved_regs)
{
    int sig = TARGET_SIGNAL_TRAP;

    switch (exc_nr) {
      case BSP_EXC_IACCESS:
      case BSP_EXC_DACCESS:
	sig = TARGET_SIGNAL_SEGV;
	break;

      case BSP_EXC_IPRIV:
      case BSP_EXC_ILL:
      case BSP_EXC_CPDIS:
      case BSP_EXC_FPDIS:
	sig = TARGET_SIGNAL_ILL;
	break;

      case BSP_EXC_ALIGN:
	sig = TARGET_SIGNAL_BUS;
	break;

      case BSP_EXC_INT1 ... BSP_EXC_INT15:
	sig = TARGET_SIGNAL_INT;
	break;
    }

    return sig;
}


/*
 * Set the PC value in the saved registers.
 */
void
bsp_set_pc(unsigned long pc, void *saved_regs)
{
    ((ex_regs_t *)saved_regs)->_pc = pc;
    ((ex_regs_t *)saved_regs)->_npc = pc + 4;
}


/*
 * Get the PC value from the saved registers.
 */
unsigned long
bsp_get_pc(void *saved_regs)
{
    return ((ex_regs_t *)saved_regs)->_pc;
}


void
bsp_copy_gdb_to_exc_regs(ex_regs_t *ex_regs, gdb_regs_t *gdb_regs)
{
    /* for sparc, our exception frame matches the gdb layout */
    memcpy(ex_regs, gdb_regs, sizeof(gdb_regs_t));
}

void
bsp_copy_exc_to_gdb_regs(gdb_regs_t *gdb_regs, ex_regs_t *ex_regs)
{
    /* for sparc, our exception frame matches the gdb layout */
    memcpy(gdb_regs, ex_regs, sizeof(gdb_regs_t));
}


/*
 * Disable interrupts for saved context. Return true if 
 * interrupts were enabled. Return value has no meaning
 * for enable function.
 */
int
bsp_set_interrupt_enable(int disable, void *saved_regs)
{
    unsigned long psr;
    static unsigned pil;

    bsp_get_register(REG_PSR, saved_regs, &psr);

    if (disable) {
	pil = psr & 0xf00;
	psr |= 0xf00;
    } else {
	psr &= ~0xf00;
	psr |= pil;
    }
    bsp_set_register(REG_PSR, saved_regs, &psr);
    return 1;
}

