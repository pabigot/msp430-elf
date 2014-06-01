/*
 * cache-sparcl.S -- Sparclite cache ops.
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

	.file "cache-sparcl.S"

	.text	
	
/*
 *  __dcache_flush -- invalidate data cache.
 *
 *  Usage: void __dcache_flush(void *addr, int nbytes); 
 */
FUNC_START __dcache_flush
	sethi	%hi(0x1000),%o0		/* bank 1 invalidate */
	sethi	%hi(0x80000000),%o1	/* bank 2 invalidate */
	mov	1,%o2			/* clear valid bits */ 
	sta	%o2,[%o0] 0xe		/* do it - dcache bank 1 */
	sta	%o2,[%o0 + %o1] 0xe	/* do it - dcache bank 2 */
	retl
	 nop
FUNC_END __dcache_flush

	
/*
 *  __icache_flush -- invalidate instruction cache.
 *
 *  Usage: void __icache_flush(void *addr, int nbytes); 
 */
FUNC_START __icache_flush
	sethi	%hi(0x1000),%o0		/* bank 1 invalidate */
	sethi	%hi(0x80000000),%o1	/* bank 2 invalidate */
	mov	1,%o2			/* clear valid bits */ 
	sta	%o2,[%o0] 0xc		/* do it - icache bank 1 */
	sta	%o2,[%o0 + %o1] 0xc	/* do it - icache bank 2 */
	nop
	nop
	nop
	nop
	retl
	 nop
FUNC_END __icache_flush

	
