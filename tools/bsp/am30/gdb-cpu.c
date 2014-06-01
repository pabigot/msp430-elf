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
#include "gdb-cpu.h"

static const int __regbyte[NUMREGS] = {
    (int)&((ex_regs_t *)0)->_d0,
    (int)&((ex_regs_t *)0)->_d1,
    (int)&((ex_regs_t *)0)->_d2,
    (int)&((ex_regs_t *)0)->_d3,

    (int)&((ex_regs_t *)0)->_a0,
    (int)&((ex_regs_t *)0)->_a1,
    (int)&((ex_regs_t *)0)->_a2,
    (int)&((ex_regs_t *)0)->_a3,

    (int)&((ex_regs_t *)0)->_sp,
    (int)&((ex_regs_t *)0)->_pc,

    (int)&((ex_regs_t *)0)->_mdr,
    (int)&((ex_regs_t *)0)->_psw,
    (int)&((ex_regs_t *)0)->_lir,
    (int)&((ex_regs_t *)0)->_lar,
};

/*
 *  Get byte offset from start of a saved regs struct
 *  to a given register. Register numbers are defined
 *  by gdb.
 */
int
bsp_regbyte(int regno)
{
    return __regbyte[regno];
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
    if (exc_nr != BSP_EXC_TRAP) {
	/*
	 *  Exceptions other than trap place the signal value
	 *  in the dummy field of the frame.
	 */
	return ((ex_regs_t *)saved_regs)->_dummy;
    }

    /* anything else is a trap */
    return TARGET_SIGNAL_TRAP;
}


/*
 * Set the PC value in the saved registers.
 */
void
bsp_set_pc(unsigned long pc, void *saved_regs)
{
    ((ex_regs_t *)saved_regs)->_pc = pc;
}


/*
 * Get the PC value from the saved registers.
 */
unsigned long
bsp_get_pc(void *saved_regs)
{
    return ((ex_regs_t *)saved_regs)->_pc;
}

