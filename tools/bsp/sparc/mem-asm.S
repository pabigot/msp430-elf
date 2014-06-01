/*
 * mem-asm.S -- SPARC alternate memory access support.
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
#include <bsp/bsp.h>
#include <bsp/cpu.h>

	.file "mem-asm.S"
	
/*
 * These routines are placed in RAM so that the ASI field of the
 * load and store alternate instructions can be modified. This is
 * a terrible way to do things, but it is necessary to support
 * memory access to arbitrary address spaces.
 */
	.data
	.align 4
	
	.globl SYM_NAME(__load_8a_insn)
	.globl SYM_NAME(__store_8a_insn)
	.globl SYM_NAME(__load_16a_insn)
	.globl SYM_NAME(__store_16a_insn)
	.globl SYM_NAME(__load_32a_insn)
	.globl SYM_NAME(__store_32a_insn)
	.globl SYM_NAME(__load_64a_insn)
	.globl SYM_NAME(__store_64a_insn)

FUNC_START __load_8a
SYM_NAME(__load_8a_insn):
	lduba	[%o0] 0,%o0
	retl
	 stb	%o0,[%o1]
FUNC_END __load_8a

FUNC_START __store_8a
	ldub	[%o0],%o0
	retl
SYM_NAME(__store_8a_insn):
	 stba	%o0,[%o1] 0
FUNC_END __store_8a

FUNC_START __load_16a
SYM_NAME(__load_16a_insn):
	lduha	[%o0] 0,%o0
	retl
	 sth	%o0,[%o1]
FUNC_END __load_16a

FUNC_START __store_16a
	lduh	[%o0],%o0
	retl
SYM_NAME(__store_16a_insn):
	 stha	%o0,[%o1] 0
FUNC_END __store_16a

FUNC_START __load_32a
SYM_NAME(__load_32a_insn):
	lda	[%o0] 0,%o0
	retl
	 st	%o0,[%o1]
FUNC_END __load_32a

FUNC_START __store_32a
	ld	[%o0],%o0
	retl
SYM_NAME(__store_32a_insn):
	 sta	%o0,[%o1] 0
FUNC_END __store_32a


