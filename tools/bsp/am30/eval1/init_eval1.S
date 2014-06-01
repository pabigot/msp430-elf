/*
 * init_eval1.S -- AM30 (MN10300) BSP hardware initialization.
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
	.file "init_eval1.S"
        .title "Boot Code for AM30 EVAL1 board"

#include <bsp/cpu.h>

	.text

        .align     2
FUNC_START _bsp_init_stack
	/*
	 *  Set up CPU and initial stack. Branch to address in a0
	 *  when done.
	 */	
	.equ	INIT_MEMCTR0,0x1200	# 2wait
	.equ	INIT_MEMCTR1,0x0120	# 1wait-32bit-
	.equ	INIT_MEMCTR2,0x0065
	.equ	INIT_DRAMCTR,0x0287
	.equ	INIT_REFCNT,0x00eb	
	.equ	INIT_P2MD,0xf0
	.equ	INIT_P2DIR,0xff
	.equ	DRAM_TOP,0x48100000

	mov	INIT_MEMCTR0,d2
	movhu	d2,(MEMCTR0)

	mov	INIT_MEMCTR1,d2
	movhu	d2,(MEMCTR1)

	mov	INIT_MEMCTR2,d2
	movhu	d2,(MEMCTR2)

	mov	INIT_REFCNT,d2
	movhu	d2,(REFCNT)

	mov	INIT_DRAMCTR,d2
	movhu	d2,(DRAMCTR)

	/* Set up LED */
	mov	INIT_P2MD,d2
	movbu	d2,(P2MD)

	mov	INIT_P2DIR,d2
	movbu	d2,(P2DIR)

	/* Set up the PSW */
	mov	0x0700,d0
	mov	d0,psw

	/* set up stack */

	mov	DRAM_TOP - 0x10,a1
	sub	8,a1
	mov	a1,sp

	/* DRAM size */
	mov	0x100000,d0
	mov	d0,(sp)

	jmp	(a0)
FUNC_END _bsp_init_stack
