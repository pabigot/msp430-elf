/*
 * gdb-cpu.c -- FR-V CPU specific support for GDB stub.
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
#include <string.h>
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
    return 4 * regno;
}


/*
 * Return size in bytes of given register.
 */
int
bsp_regsize(int regno)
{
    return sizeof(int);
}


/*
 *  Given a hw exception number and a pointer to saved registers,
 *  return a GDB signal value.
 */
int
bsp_get_signal(int exc_nr, void *saved_regs)
{
    int sig = TARGET_SIGNAL_TRAP;

    switch (exc_nr) {
      case TT_MP_DISABLED:
      case TT_MP_EXCEPTION:
      case TT_COMMIT:
      case TT_COMPOUND:
	break;

      case TT_REG_EXCEPTION:
      case TT_ILLEGAL:
      case TT_PRIVILEGE:
	sig = TARGET_SIGNAL_ILL;
	break;

      case TT_DSTORE_ERROR:
      case TT_MEM_ALIGN:
	sig = TARGET_SIGNAL_BUS;
	break;

      case TT_DACCESS_ERROR:
      case TT_DACCESS_MMU:
      case TT_DACCESS_EXCEPTION:
      case TT_IACCESS_MMU:
      case TT_IACCESS_ERROR:
      case TT_IACCESS_EXCEPTION:
	sig = TARGET_SIGNAL_SEGV;
	break;

      case TT_DIVISION:
      case TT_FP_DISABLED:
      case TT_FP_EXCEPTION:
	sig = TARGET_SIGNAL_FPE;
	break;

      case TT_INT1 ... TT_INT15:
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
    ex_regs_t *regs = (ex_regs_t *)saved_regs;
    regs->_pc = pc;
    if (regs->_trap == 0xff)
	regs->_bpcsr = pc;
    else
	regs->_pcsr = pc;
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
    /* for now, our exception frame matches the gdb layout */
    memcpy(ex_regs, gdb_regs, sizeof(gdb_regs_t));
}

void
bsp_copy_exc_to_gdb_regs(gdb_regs_t *gdb_regs, ex_regs_t *ex_regs)
{
    /* for now, our exception frame matches the gdb layout */
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
    unsigned int psr;
    static unsigned pil;

    bsp_get_register(REG_PSR, saved_regs, &psr);

    if (disable) {
	pil = psr & PSR_PIL_MASK;
	psr |= (15 << PSR_PIL_SHIFT);
    } else {
	psr &= ~PSR_PIL_MASK;
	psr |= pil;
    }
    bsp_set_register(REG_PSR, saved_regs, &psr);
    return 1;
}
