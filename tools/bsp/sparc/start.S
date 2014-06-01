/*
 * start.S -- SPARC BSP startup.
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

        .file   "start.S"

        .text

        /*
         * The hardware reset vector jumps to here.
         */
FUNC_START start
        wr      %g0,PSR_INIT,%psr
        wr      %g0,1,%wim
        wr      %g0,0,%tbr

#if defined(SINGLE_VECTOR_TRAP)
#error SINGLE_VECTOR_TRAP not yet implemented.
        wr      %g0, 1, %asr17
#endif
        /*
         * Minimally initialize cpu and board.
         * Establish the initial stack.
         */
        call    SYM_NAME(_bsp_init_stack)
        nop
        mov     %sp,%fp
        
	mov     %sp,%o0
        call    SYM_NAME(c_rom_start)
        nop
FUNC_END start
