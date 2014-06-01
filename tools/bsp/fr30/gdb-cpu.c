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
 * Return byte offset within the saved register area of the
 * given register.
 */
int
bsp_regbyte(int regno)
{
    switch(regno)
    {
      case REG_R0:
	return (int)&((ex_regs_t *)0)->_r0;
      case REG_R1:
	return (int)&((ex_regs_t *)0)->_r1;
      case REG_R2:
	return (int)&((ex_regs_t *)0)->_r2;
      case REG_R3:
	return (int)&((ex_regs_t *)0)->_r3;
      case REG_R4:
	return (int)&((ex_regs_t *)0)->_r4;
      case REG_R5:
	return (int)&((ex_regs_t *)0)->_r5;
      case REG_R6:
	return (int)&((ex_regs_t *)0)->_r6;
      case REG_R7:
	return (int)&((ex_regs_t *)0)->_r7;
      case REG_R8:
	return (int)&((ex_regs_t *)0)->_r8;
      case REG_R9:
	return (int)&((ex_regs_t *)0)->_r9;
      case REG_R10:
	return (int)&((ex_regs_t *)0)->_r10;
      case REG_R11:
	return (int)&((ex_regs_t *)0)->_r11;
      case REG_R12:
	return (int)&((ex_regs_t *)0)->_r12;
      case REG_R13:
	return (int)&((ex_regs_t *)0)->_r13;
      case REG_R14:
	return (int)&((ex_regs_t *)0)->_r14;
      case REG_R15:
	return (int)&((ex_regs_t *)0)->_r15;
      case REG_PC:
	return (int)&((ex_regs_t *)0)->_pc;
      case REG_PS:
	return (int)&((ex_regs_t *)0)->_ps;
      case REG_TBR:
	return (int)&((ex_regs_t *)0)->_tbr;
      case REG_RP:
	return (int)&((ex_regs_t *)0)->_rp;
      case REG_SSP:
	return (int)&((ex_regs_t *)0)->_ssp;
      case REG_USP:
	return (int)&((ex_regs_t *)0)->_usp;
      case REG_MDH:
	return (int)&((ex_regs_t *)0)->_mdh;
      case REG_MDL:
	return (int)&((ex_regs_t *)0)->_mdl;
    }
    return 0;
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
      case BSP_EXC_NMI ... BSP_EXC_DELAY:
	sig = TARGET_SIGNAL_INT;
	break;

      case BSP_EXC_ILL:
	sig = TARGET_SIGNAL_ILL;
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
}


/*
 * Get the PC value from the saved registers.
 */
unsigned long
bsp_get_pc(void *saved_regs)
{
    return ((ex_regs_t *)saved_regs)->_pc;
}


