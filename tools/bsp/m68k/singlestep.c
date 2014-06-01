/*
 * singlestep.c -- M68K specific single-step support.
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

/*
 *  Cleanup after a singlestep.
 */
void
bsp_singlestep_cleanup(void *saved_regs)
{
  /*
   * Clear the trace bit to disable single stepping
   */
  ((ex_regs_t *)saved_regs)->_sr &= ~MC68000_SR_TRACEMODE_ANY;
}


/*
 * Set breakpoint instructions for single stepping.
 */
void
bsp_singlestep_setup(void *saved_regs)
{
  /*
   * Set the trace bit to enable single stepping
   */
  ((ex_regs_t *)saved_regs)->_sr |= MC68000_SR_TRACEMODE_ANY;
}


void
bsp_skip_instruction(void *saved_regs)
{
    ((ex_regs_t *)saved_regs)->_pc += GDB_BREAKPOINT_INST_SIZE;
}
