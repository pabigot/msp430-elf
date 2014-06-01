/*
 * cache8xx.S -- PowerPC MPC8xx cache ops.
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

	.file "cache8xx.S"

	.text	
	
/*
 * Instruction and Data Cache definitions.
 * NB: must use with lis instruction to load into bit 0-15.
 */
#define CACHE_UNLOCKALL		0x0A00
#define CACHE_DISABLE		0X0400
#define CACHE_INVALIDATEALL	0X0C00
#define CACHE_ENABLE		0X0200
#define CACHE_ENABLEBIT		0X8000

/*
 *  __dcache_flush -- write data cache to main memory and invalidate.
 *
 *  Usage: void __dcache_flush(void *addr, int nbytes); 
 */
FUNC_START __dcache_flush
#if USE_DCACHE
#if 0
	andi.	r5,r3,15	/* get addr&15 */
	xor	r3,r3,r5        /* force addr to cache line boundary */
	add	r4,r4,r5	/* adjust length */
	addi	r4,r4,15
	srwi	r4,r4,4		/* number of cache lines to flush */
	li	r0,0		/* index */
    1:
        addic.  r4,r4,-1	/* decrementer, set cc bit */
	dcbf	r0,r3
	addi	r3,r3,16
	bgt	1b
#else
	li	r3,0	
	li	r0,0	
	li	r4,256		/* number of cache lines to flush */
    1:
        addic.  r4,r4,-1	/* decrementer, set cc bit */
        lwzx    r5,r0,r3
	dcbf	r0,r3
	addi	r3,r3,16
	bgt	1b
#endif
#endif
	blr
FUNC_END __dcache_flush


/*
 *  __dcache_disable -- Disable data cache.
 *
 *  Usage: int __dcache_disable(void)
 *		return non-zero if cache was enabled,
 *		zero if already disabled
 */
FUNC_START __dcache_disable
#if USE_DCACHE
	mfspr	r4,dc_cst		/* clear error bits, get enable bit */
        andis.  r4,r4,CACHE_ENABLEBIT
        beq     8f			/* skip flush & disable if already disabled */

        lis     r3,CACHE_UNLOCKALL
	sync
        mtspr   dc_cst,r3		/* unlock all lines */
	
	li	r3,0	
	li	r0,0	
	li	r4,256		/* number of cache lines to flush */
    1:
        addic.  r4,r4,-1	/* decrementer, set cc bit */
        lwzx    r5,r0,r3
	dcbf	r0,r3
	addi	r3,r3,16
	bgt	1b

#if 0
        lis     r3,CACHE_INVALIDATEALL
        sync
        mtspr   dc_cst,r3		/* disable dcache */
#endif
	
        lis     r3,CACHE_DISABLE
        sync
        mtspr   dc_cst,r3		/* disable dcache */
	li	r4,1
    8:
	mr	r3,r4
#else
	li	r3,0
#endif
	blr
FUNC_END __dcache_disable
	
	
	
/*
 *  dcache_enable -- Enable data cache.
 */	
FUNC_START __dcache_enable
        lis     r3,CACHE_ENABLE
        sync
        mtspr   dc_cst,r3
        sync
	blr
FUNC_END __dcache_enable
	
	
/*
 *  __icache_flush -- invalidate instruction cache.
 *
 *  Usage: void __icache_flush(void *addr, int nbytes); 
 */
FUNC_START __icache_flush
#if USE_ICACHE
#if 0
	andi.	r5,r3,15	/* get addr&15 */
	xor	r3,r3,r5        /* force addr to cache line boundary */
	add	r4,r4,r5	/* adjust length */
	addi	r4,r4,15
	srwi	r4,r4,4		/* number of cache lines to flush */
	li	r0,0		/* index */
    1:
        addic.  r4,r4,-1	/* decrementer, set cc bit */
	icbi	r0,r3
	addi	r3,r3,16
	bgt	1b
#else
	mfspr	r3,ic_cst		/* clear error bits */
	lis	r3,CACHE_DISABLE
	mtspr	ic_cst,r3               /* disable */
	sync
        lis     r3,CACHE_UNLOCKALL
        mtspr   ic_cst,r3		/* unlock all lines */
        lis     r3,CACHE_INVALIDATEALL
        mtspr   ic_cst,r3		/* invalidate all lines */
        isync
        lis     r3,CACHE_ENABLE
        mtspr   ic_cst,r3
        isync
#endif
#endif
	blr
FUNC_END __icache_flush

	
/*
 *  __icache_disable -- Disable instruction cache.
 */	
FUNC_START __icache_disable
	mfspr	r3,ic_cst		/* clear error bits */
	lis	r3,CACHE_DISABLE
	mtspr	ic_cst,r3               /* disable */
	sync
        lis     r3,CACHE_UNLOCKALL
        mtspr   ic_cst,r3		/* unlock all lines */
        lis     r3,CACHE_INVALIDATEALL
        mtspr   ic_cst,r3		/* invalidate all lines */
        isync
	blr
FUNC_END __icache_disable
	
	
/*
 *  __icache_enable -- Enable instruction cache.
 */	
FUNC_START __icache_enable
        lis     r3,CACHE_ENABLE
        mtspr   ic_cst,r3
        isync
	blr
FUNC_END __icache_enable
	


