/*
 * cache6xx.S -- PowerPC PPC6xx cache ops.
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

	.file "cache4xx.S"

	.text	
	
/*
 *  __dcache_flush -- write data cache to main memory and invalidate.
 *
 *  Usage: void __dcache_flush(void *addr, int nbytes); 
 */
FUNC_START __dcache_flush
	rlwinm	r6,r3,5,27,31	/* get cache region from address */
	lis	r5,0x8000
	srw	r5,r5,r6	/* get mask for cacheability test */
	mfdccr	r6		/* get cacheability enables */
	and.	r6,r6,r5	/* test for cacheability */
	beq	9f		/* branch and do nothing if not cached */
	/* must flush */	
	andi.	r5,r3,15	/* get addr&15 */
	xor	r3,r3,r5        /* force addr to cache line boundary */
	add	r4,r4,r5	/* adjust length */
	addi	r4,r4,15
	srwi	r4,r4,4		/* number of cache lines to flush */
	li	r0,0		/* index */
    1:
        addic.  r4,r4,-1	/* decrementer, set cc bit */
	dcbf	r3,r0
	addi	r3,r3,16
	bgt	1b
    9:
	sync
	blr
FUNC_END __dcache_flush

	
/*
 *  __icache_flush -- invalidate instruction cache.
 *
 *  Usage: void __icache_flush(void *addr, int nbytes); 
 */
FUNC_START __icache_flush
	rlwinm	r6,r3,5,27,31	/* get cache region from address */
	lis	r5,0x8000
	srw	r5,r5,r6	/* get mask for cacheability test */
	mficcr	r6		/* get cacheability enables */
	and.	r6,r6,r5	/* test for cacheability */
	beq	9f		/* branch and do nothing if not cached */
	/* must flush */	
	andi.	r5,r3,15	/* get addr&15 */
	xor	r3,r3,r5        /* force addr to cache line boundary */
	add	r4,r4,r5	/* adjust length */
	addi	r4,r4,15
	srwi	r4,r4,4		/* number of cache lines to flush */
	li	r0,0		/* index */
    1:
        addic.  r4,r4,-1	/* decrementer, set cc bit */
	icbi	r3,r0
	addi	r3,r3,16
	bgt	1b
    9:
        isync
	blr
FUNC_END __icache_flush
