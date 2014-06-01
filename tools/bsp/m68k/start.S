/*
 * start.S -- ROM-based startup for M68K
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
        .file   "start.S"

        .text

        /* Reset Supervisor Stack Pointer */
        .long SUPERSTACK

        /* Reset Initial Program Counter */
        .long SYM_NAME(start)

FUNC_START start
        /*
         * Disable all interrupts
         */
        movew   IMM(0x2700), sr

#ifdef __CPU_MC68328__
        /*
         * Dragonball Register base will be in a5
         */
        moveal  IMM(MC68328_REG_BASE), a5

        /*
         * Enable bus error timeouts
         */
        bsetb   IMM(4), a5@(MC68328_SCR_o)
        
        /*
         * Disable the watchdog timer.
         */
        movew   IMM(0), a5@(MC68328_WDOG_CTL_o)
#endif /* __CPU_MC68328__ */
        
        /*
         *  Jump to _bsp_init_stack with return address in a0.
         *  We call _bsp_init_stack this way because there is
         *  no stack, yet.
         */
        moveal  IMM(SYM_NAME(c_rom_start)), a0
        jmp     SYM_NAME(_bsp_init_stack)
FUNC_END _start
