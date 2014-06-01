/*
 * crt0.S -- startup file for PowerPC systems.
 *
 * Copyright (c) 1995, 1999 Cygnus Support
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

	.file	"crt0.S"
		
FUNC_START start
.Lnostack:
	/* set up initial stack frame */
	li	r0,63
	andc	r1,r1,r0
	lis	r0,0
	stw	r0,0(sp)		/* clear back chain */
	stwu	sp,-64(sp)		/* push another stack frame */

	bl	SYM_NAME(c_start)
FUNC_END start

	.extern	SYM_NAME(atexit)
	.globl	SYM_NAME(__atexit)
	.section ".sdata","aw"
	.align	2
SYM_NAME(__atexit):			/* tell C's eabi-ctor's we have an atexit function */
	.long	SYM_NAME(atexit)@fixup	/* and that it is to register __do_global_dtors */

	.section ".fixup","aw"
	.align	2
	.long	SYM_NAME(__atexit)
