/*
 * start.S -- FR-V BSP startup.
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
#include "fr500.h"

        .file "start.S"

        .text

        /*
         * The hardware reset vector jumps to here.
         */
FUNC_START start
	movsg	psr,gr4
	seti	~(PSR_ICE|PSR_SE|PSR_ESR|PSR_ECS|PSR_EC|PSR_ET),gr5
	and	gr4,gr5,gr6
	seti	(PSR_NEM|PSR_CM|PSR_EF|PSR_EM|PSR_S|PSR_PS|PSR_ET),gr5
	or	gr6,gr5,gr4
	movgs	gr4,psr

	/* set TBR to start of ROM */
	call	1f
    1:
	movsg	lr,gr4
	srli	gr4,16,gr4
	slli	gr4,16,gr4
	movgs	gr4,tbr
	
        /*
         * Minimally initialize cpu and board.
         * Establish the initial stack.
         */
        call    SYM_NAME(_bsp_init_stack)
	
	setlos	#0, fp		/* zero fp to allow unwinders to stop */

	ld	@(sp,gr0),gr8
        call    SYM_NAME(c_rom_start)
    1:
	bra	1b
FUNC_END start




