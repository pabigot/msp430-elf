/*
 * reset.S -- PowerPC BSP reset vector.
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

	.file "reset.S"

	.extern	SYM_NAME(_bsp_init_stack)
	.extern	SYM_NAME(common_start)
	.extern	SYM_NAME(__ci_handler)

#ifdef __BIG_ENDIAN__
#  define MSR_ENDIAN 0
#else
#  define MSR_ENDIAN MSR_LE|MSR_ILE
#endif

#ifdef __HAVE_FPU__
#  define INIT_MSR  (MSR_FP|MSR_ME|MSR_FE0|MSR_FE1|MSR_IP|MSR_RI|MSR_ENDIAN)
#else
#  define INIT_MSR  (MSR_ME|MSR_IP|MSR_RI|MSR_ENDIAN)
#endif

	/*
	 * The only ROM vector is the hard reset vector.
	 * We don't need this if we're switching to little endian
	 * and this module is to be used with soft reset.
	 */
#if !defined(__SWITCH_TO_LE__) || !defined(__LITTLE_ENDIAN__)
	.section ".rom_vectors","ax"
	/*
	 *  Reserved vector 0
	 */
	nop

	.p2align 8
	/* Offset 0x100 */
#if defined(__CPU_PPC4XX__)
	/*
	 * Critical Interrupt
	 */
	mtspr	sprg0,r3
	mflr	r3
	mtspr	sprg1,r3
	lis	r3,SYM_NAME(__ci_handler)@h
	ori	r3,r3,SYM_NAME(__ci_handler)@l
	mtlr	r3
	blrl
#else
	/*
	 *  Reset vector
	 */
	lis	r3,SYM_NAME(start)@h
	ori	r3,r3,SYM_NAME(start)@l
	mtlr	r3
	blr
#endif
#endif

	.text
	/*
	 * This module is linked first and if its the little endian
	 * part of a __SWITCH_TO_LE__ ROM, then the first thing must jump
	 * to common_start. This will be the first code executed
	 * after the switch to LE.
	 */
#if defined(__SWITCH_TO_LE__) && defined(__LITTLE_ENDIAN__)
	b	SYM_NAME(common_start)
#endif
	
FUNC_START start
#if !defined(__CPU_PPC4XX__)
	/* Set up MSR */
	lis	r3,INIT_MSR@h
	ori	r3,r3,INIT_MSR@l
	mtmsr	r3
        mtspr	srr1,r3
#endif

	/* set up time base register to zero */
	xor	r3,r3,r3
	mtspr	tblw,r3
	xor	r4,r4,r4
	mtspr	tbuw,r4
	
#if defined(__CPU_MPC8XX__)
#define CACHE_UNLOCKALL		0x0A00
#define CACHE_DISABLE		0X0400
#define CACHE_INVALIDATEALL	0X0C00
#define CACHE_ENABLE		0X0200
#define CACHE_ENABLEBIT		0X8000

	mfspr	r3,dc_cst		/* clear error bits */
        lis     r3,CACHE_UNLOCKALL
	sync
        mtspr   dc_cst,r3		/* unlock all lines */
	
        lis     r3,CACHE_INVALIDATEALL
	sync
        mtspr   dc_cst,r3		/* invalidate all lines */

	lis	r3,CACHE_DISABLE
	sync
	mtspr	dc_cst,r3               /* disable */
	
	
	mfspr	r3,ic_cst		/* clear error bits */
        lis     r3,CACHE_UNLOCKALL
        mtspr   ic_cst,r3		/* unlock all lines */
	isync
        lis     r3,CACHE_INVALIDATEALL
        mtspr   ic_cst,r3		/* invalidate all lines */
        isync
	lis	r3,CACHE_DISABLE
	mtspr	ic_cst,r3               /* disable */
	isync
	
	sync
	
	/*
	 * Debug related registers.
	 */
        lwi	r3,0x00000007
        mtspr	ictrl,r3

        lwi	r3,0x00000000
        mtspr	der,r3
#endif
	
	/*
	 * Call _bsp_init_stack to do just enough initialization to
	 * set up clocks, chip selects, and give us a stack.
	 * Return address is setup to point to common_start.
	 * If switching to little endian, we have to link this
	 * separately and use a hard coded address to separately
	 * linked little-endian code.
	 */
#if defined(__SWITCH_TO_LE__) && defined(__BIG_ENDIAN__)
	lis	r3,0xfff0
	ori	r3,r3,0x1000
#else
	lis	r3,SYM_NAME(common_start)@h
	ori	r3,r3,SYM_NAME(common_start)@l
#endif
	mtlr	r3
	b	SYM_NAME(_bsp_init_stack)
FUNC_END start
