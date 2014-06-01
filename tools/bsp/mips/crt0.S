/*
 * crt0.S -- startup file for MIPS.
 *
 * Copyright (c) 1995, 1996, 1997, 1998, 1999, 2000 Cygnus Support
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

#ifdef __mips16
	/* This file contains 32 bit assembly code.  */
	.set nomips16
#endif

	.file 1 "bsp-crt0.S"
	
	.text
	.p2align 2

FUNC_START start
#ifndef __R3000
#ifdef __USE_64BIT_MODE
	mfc0	t0,C0_STATUS	/* set initial SR */
	li	t1,SR_KX|SR_SX|SR_UX
	or	t0,t0,t1
	mtc0	t0,C0_STATUS	/* set initial SR */
#endif
#endif
	/* set the global pointer */
	la	gp,SYM_NAME(_gp)
	jal	SYM_NAME(c_start)
FUNC_END start
