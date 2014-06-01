/*
 * start.S -- PowerPC BSP startup.
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
#include <bsp/bsp.h>
#include <bsp/cpu.h>

	.file "start.S"

	.extern	SYM_NAME(_bsp_init)
	.extern	SYM_NAME(__eabi)
	.extern SYM_NAME(start)

#if !defined(__CPU_PPC4XX__)
#  if defined(__HAVE_FPU__)
#    define EXC_MSR   (MSR_FP|MSR_ME|MSR_FE0|MSR_FE1|MSR_RI)
#  else /* defined(__HAVE_FPU__) */
#    define EXC_MSR   (MSR_ME|MSR_RI)
#  endif /* defined(__HAVE_FPU__) */
#endif /* !defined(__CPU_PPC4XX__) */

	.text

FUNC_START common_start
	/***************************************************
	 * RAM Vector Initialization
	 *
	 *    The user exeception vector will be initialized
	 *    and located at 0 so that user can claim 
	 *    every exception except trace for the application.
	 *
	 *****************************************************/
        lis     r5,0x0000
        lis     r4,SYM_NAME(__vectors_start)@h
        ori     r4,r4,SYM_NAME(__vectors_start)@l
        li      r3,0x1000
    1:
        addic.  r3,r3,-1
        lwz     r6,0(r4)
        addi    r4,r4,4
        stw     r6,0(r5)
        addi    r5,r5,4
        bne     1b

	/* Change exception prefix to 0 so that CPU will
	   jump to vector table at 0 instead of FFF00000 */
#if defined(__CPU_PPC4XX__)
	xor	r0,r0,r0
	mtspr	evpr,r0
#else
        mfmsr   r3
	lwi	r4,0xffffffbf
        and	r3,r3,r4
        mtmsr   r3
#endif

	li	r0,0
	stw	r0,0(sp) /* zero backlink */

        /*
         *  Call __eabi to set up runtime environment. This
         *  is normally called from the prologue in main(),
         *  but we do it here so that _bsp_init can be
         *  written in C or C++.
         */
	stwu	sp,-UNDERHEAD_SIZE(sp)
        /*bl	SYM_NAME(__eabi)*/

	li	r0,0
	li	r3,SYM_NAME(__atexit)@h
	ori	r3,r3,SYM_NAME(__atexit)@l
	stw	r0,0(r3)

	lwz	r3,0(sp)
	bl	SYM_NAME(c_rom_start)
FUNC_END common_start

.macro default_vec
	mtspr	sprg0,r3
	mflr	r3
	mtspr	sprg1,r3
	lis	r3,SYM_NAME(__exception_handler)@h
	ori	r3,r3,SYM_NAME(__exception_handler)@l
	mtlr	r3
	blrl
.endm
		
	.p2align 8
SYM_NAME(__vectors_start):
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
	.p2align 8
	/* Offset 0x200 */
#if defined(__CPU_PPC4XX__)
	/*
	 * Machine Check
	 */
	mtspr	sprg0,r3
	mflr	r3
	mtspr	sprg1,r3
	lis	r3,SYM_NAME(__ci_handler)@h
	ori	r3,r3,SYM_NAME(__ci_handler)@l
	mtlr	r3
	blrl
#else
	default_vec
#endif
	
	.p2align 8
	/* Offset 0x300 */
	default_vec
	
	.p2align 8
	/* Offset 0x400 */
	default_vec
	
	.p2align 8
	/* Offset 0x500 */
	default_vec
	
	.p2align 8
	/* Offset 0x600 */
	default_vec
	
	.p2align 8
	/* Offset 0x700 */
	default_vec
	
	.p2align 8
	/* Offset 0x800 */
	default_vec
	
	.p2align 8
	/* Offset 0x900 */
	default_vec
	
	.p2align 8
	/* Offset 0xA00 */
	default_vec
	
	.p2align 8
	/* Offset 0xB00 */
	default_vec
	
	.p2align 8
	/* Offset 0xC00 */
	default_vec
	
	.p2align 8
	/* Offset 0xD00 */
	default_vec
	
	.p2align 8
	/* Offset 0xE00 */
	default_vec
	
	.p2align 8
	/* Offset 0xF00 */
	default_vec
	
	.p2align 8
	/* Offset 0x1000 */
#if defined(__CPU_PPC4XX__)
	/* pit */
	b	1f
	nop
	nop
	nop
	/* fit */
	b	2f
	nop
	nop
	nop
	/* watchdog */
	mtspr	sprg0,r3
	mflr	r3
	mtspr	sprg1,r3
	lis	r3,SYM_NAME(__wdt_handler)@h
	ori	r3,r3,SYM_NAME(__wdt_handler)@l
	mtlr	r3
	blr
    1:
	mtspr	sprg0,r3
	mflr	r3
	mtspr	sprg1,r3
	lis	r3,SYM_NAME(__pit_handler)@h
	ori	r3,r3,SYM_NAME(__pit_handler)@l
	mtlr	r3
	blr
    2:
	mtspr	sprg0,r3
	mflr	r3
	mtspr	sprg1,r3
	lis	r3,SYM_NAME(__fit_handler)@h
	ori	r3,r3,SYM_NAME(__fit_handler)@l
	mtlr	r3
	blr
#else
	default_vec
#endif
	
	.p2align 8
	/* Offset 0x1100 */
	default_vec
	
	.p2align 8
	/* Offset 0x1200 */
	default_vec
	
	.p2align 8
	/* Offset 0x1300 */
	default_vec
	
	.p2align 8
	/* Offset 0x1400 */
	default_vec
	
	.p2align 8
	/* Offset 0x1500 */
	default_vec
	
	.p2align 8
	/* Offset 0x1600 */
	default_vec
	
	.p2align 8
	/* Offset 0x1700 */
	default_vec
	
	.p2align 8
	/* Offset 0x1800 */
	default_vec
	
	.p2align 8
	/* Offset 0x1900 */
	default_vec
	
	.p2align 8
	/* Offset 0x1A00 */
	default_vec
	
	.p2align 8
	/* Offset 0x1B00 */
	default_vec
	
	.p2align 8
	/* Offset 0x1C00 */
	default_vec
	
	.p2align 8
	/* Offset 0x1D00 */
	default_vec
	
	.p2align 8
	/* Offset 0x1E00 */
	default_vec
	
	.p2align 8
	/* Offset 0x1F00 */
	default_vec
	
	.p2align 8
	/* Offset 0x2000 */
#if defined(__CPU_PPC4XX__)
	/*
	 * Debug Exception
	 */
	mtspr	sprg0,r3
	mflr	r3
	mtspr	sprg1,r3
	lis	r3,SYM_NAME(__ci_handler)@h
	ori	r3,r3,SYM_NAME(__ci_handler)@l
	mtlr	r3
	blrl
#else
	default_vec
#endif
	
	/*
	 *  The remaining vectors use the same indirection scheme
	 *  so that even vectors in ROM can be overridden.
	 */
	.rept 31
	.p2align 8
	default_vec
	.endr

	.p2align 8
  SYM_NAME(__vectors_end):

	.macro SAVE_GPRS
	stw	r2,GPR2(sp)
	stw	r3,GPR3(sp)
	stw	r4,GPR4(sp)
	stw	r5,GPR5(sp)
	stw	r6,GPR6(sp)
	stw	r7,GPR7(sp)
	stw	r8,GPR8(sp)
	stw	r9,GPR9(sp)
	stw	r10,GPR10(sp)
	stw	r11,GPR11(sp)
	stw	r12,GPR12(sp)
	stw	r13,GPR13(sp)
	stw	r14,GPR14(sp)
	stw	r15,GPR15(sp)
	stw	r16,GPR16(sp)
	stw	r17,GPR17(sp)
	stw	r18,GPR18(sp)
	stw	r19,GPR19(sp)
	stw	r20,GPR20(sp)
	stw	r21,GPR21(sp)
	stw	r22,GPR22(sp)
	stw	r23,GPR23(sp)
	stw	r24,GPR24(sp)
	stw	r25,GPR25(sp)
	stw	r26,GPR26(sp)
	stw	r27,GPR27(sp)
	stw	r28,GPR28(sp)
	stw	r29,GPR29(sp)
	stw	r30,GPR30(sp)
	stw	r31,GPR31(sp)
	.endm

	.macro RESTORE_GPRS
	lwz	r2,GPR2(sp)
	lwz	r3,GPR3(sp)
	lwz	r4,GPR4(sp)
	lwz	r5,GPR5(sp)
	lwz	r6,GPR6(sp)
	lwz	r7,GPR7(sp)
	lwz	r8,GPR8(sp)
	lwz	r9,GPR9(sp)
	lwz	r10,GPR10(sp)
	lwz	r11,GPR11(sp)
	lwz	r12,GPR12(sp)
	lwz	r13,GPR13(sp)
	lwz	r14,GPR14(sp)
	lwz	r15,GPR15(sp)
	lwz	r16,GPR16(sp)
	lwz	r17,GPR17(sp)
	lwz	r18,GPR18(sp)
	lwz	r19,GPR19(sp)
	lwz	r20,GPR20(sp)
	lwz	r21,GPR21(sp)
	lwz	r22,GPR22(sp)
	lwz	r23,GPR23(sp)
	lwz	r24,GPR24(sp)
	lwz	r25,GPR25(sp)
	lwz	r26,GPR26(sp)
	lwz	r27,GPR27(sp)
	lwz	r28,GPR28(sp)
	lwz	r29,GPR29(sp)
	lwz	r30,GPR30(sp)
	lwz	r31,GPR31(sp)
	.endm

/*
 * Common (mostly) exception handler.
 * On entry:
 *     sprg0 = original R3
 *     sprg1 = original LR
 *     LR    = return address of vector so vec number
 *	       can be calculated.
 */	
FUNC_START __exception_handler
	mtspr	sprg2,sp
	mfcr	r3
	mtspr	sprg3,r3
	lis	r3,SYM_NAME(_ex_stack)@h
	ori	r3,r3,SYM_NAME(_ex_stack)@l
	cmplw	sp,r3
	blt	1f
	mr	sp,r3
    1:
	subi	sp,sp,EX_STACK_SIZE
	stw	r0,GPR0(sp)
	mfspr	r0,sprg2
	stw	r0,GPR1(sp)
	mfspr	r3,sprg0
	SAVE_GPRS	
        mfspr   r3,sprg1
	stw	r3,LR(sp)
	mfspr	r3,srr0
	stw	r3,NIP(sp)
	mfctr	r3
	stw	r3,CTR(sp)
	mfspr	r3,sprg3
	stw	r3,CCR(sp)
	mfxer	r3
	stw	r3,XER(sp)
	mfspr	r3,srr1
	stw	r3,MSR(sp)
	li	r3,0
	stw	r3,MQ(sp)
#ifdef EXC_MSR
	lwi	r4,EXC_MSR
	mfmsr	r3
	or	r3,r3,r4
	mtmsr	r3
#endif
#ifdef __HAVE_FPU__
	stfd	f0,FPR0+(0*8)(sp)
	stfd	f1,FPR0+(1*8)(sp)
	stfd	f2,FPR0+(2*8)(sp)
	stfd	f3,FPR0+(3*8)(sp)
	stfd	f4,FPR0+(4*8)(sp)
	stfd	f5,FPR0+(5*8)(sp)
	stfd	f6,FPR0+(6*8)(sp)
	stfd	f7,FPR0+(7*8)(sp)
	stfd	f8,FPR0+(8*8)(sp)
	stfd	f9,FPR0+(9*8)(sp)
	stfd	f10,FPR0+(10*8)(sp)
	stfd	f11,FPR0+(11*8)(sp)
	stfd	f12,FPR0+(12*8)(sp)
	stfd	f13,FPR0+(13*8)(sp)
	stfd	f14,FPR0+(14*8)(sp)
	stfd	f15,FPR0+(15*8)(sp)
	stfd	f16,FPR0+(16*8)(sp)
	stfd	f17,FPR0+(17*8)(sp)
	stfd	f18,FPR0+(18*8)(sp)
	stfd	f19,FPR0+(19*8)(sp)
	stfd	f20,FPR0+(20*8)(sp)
	stfd	f21,FPR0+(21*8)(sp)
	stfd	f22,FPR0+(22*8)(sp)
	stfd	f23,FPR0+(23*8)(sp)
	stfd	f24,FPR0+(24*8)(sp)
	stfd	f25,FPR0+(25*8)(sp)
	stfd	f26,FPR0+(26*8)(sp)
	stfd	f27,FPR0+(27*8)(sp)
	stfd	f28,FPR0+(28*8)(sp)
	stfd	f29,FPR0+(29*8)(sp)
	stfd	f30,FPR0+(30*8)(sp)
	stfd	f31,FPR0+(31*8)(sp)
#endif
	mflr	r3
	rlwinm	r3,r3,24,25,31
	stw	r3,TRAP(sp)
	mr	r4,sp
	bl	SYM_NAME(_bsp_exc_dispatch)

	/*
	 * need to call debug stub if exception not handled
	 * by application code.
	 */
	cmpwi	r3,0
	bne	1f
	lwz	r3,TRAP(sp)
	mr	r4,sp
	lis	r5,SYM_NAME(_bsp_dbg_vector)@h
	ori	r5,r5,SYM_NAME(_bsp_dbg_vector)@l
	lwz	r5,0(r5)
	mtlr	r5
	blrl
    1:
#ifdef __HAVE_FPU__
	lfd	f0,FPR0+(0*8)(sp)
	lfd	f1,FPR0+(1*8)(sp)
	lfd	f2,FPR0+(2*8)(sp)
	lfd	f3,FPR0+(3*8)(sp)
	lfd	f4,FPR0+(4*8)(sp)
	lfd	f5,FPR0+(5*8)(sp)
	lfd	f6,FPR0+(6*8)(sp)
	lfd	f7,FPR0+(7*8)(sp)
	lfd	f8,FPR0+(8*8)(sp)
	lfd	f9,FPR0+(9*8)(sp)
	lfd	f10,FPR0+(10*8)(sp)
	lfd	f11,FPR0+(11*8)(sp)
	lfd	f12,FPR0+(12*8)(sp)
	lfd	f13,FPR0+(13*8)(sp)
	lfd	f14,FPR0+(14*8)(sp)
	lfd	f15,FPR0+(15*8)(sp)
	lfd	f16,FPR0+(16*8)(sp)
	lfd	f17,FPR0+(17*8)(sp)
	lfd	f18,FPR0+(18*8)(sp)
	lfd	f19,FPR0+(19*8)(sp)
	lfd	f20,FPR0+(20*8)(sp)
	lfd	f21,FPR0+(21*8)(sp)
	lfd	f22,FPR0+(22*8)(sp)
	lfd	f23,FPR0+(23*8)(sp)
	lfd	f24,FPR0+(24*8)(sp)
	lfd	f25,FPR0+(25*8)(sp)
	lfd	f26,FPR0+(26*8)(sp)
	lfd	f27,FPR0+(27*8)(sp)
	lfd	f28,FPR0+(28*8)(sp)
	lfd	f29,FPR0+(29*8)(sp)
	lfd	f30,FPR0+(30*8)(sp)
	lfd	f31,FPR0+(31*8)(sp)
#endif
	lwz	r3,MSR(sp)
	mtspr   srr1,r3
	lwz	r3,NIP(sp)
	mtspr   srr0,r3
	lwz	r3,XER(sp)
	mtspr   xer,r3
	lwz	r3,CCR(sp)
	mtcr    r3
	lwz	r3,CTR(sp)
	mtctr   r3
	lwz	r3,LR(sp)
	mtlr	r3
	RESTORE_GPRS	
	lwz	r0,GPR0(sp)
	lwz	sp,GPR1(sp)
	isync
	sync
	rfi
FUNC_END __exception_handler
	

	
#if defined(__CPU_PPC4XX__)
/*
 * PPC 403 systems have some vector overloading which we have
 * handle differently.
 */

/*
 * Critical interrupt uses srr2 and srr3.
 */
FUNC_START __ci_handler
	mtspr	sprg2,sp
	mfcr	r3
	mtspr	sprg3,r3
	lis	r3,SYM_NAME(_ex_stack)@h
	ori	r3,r3,SYM_NAME(_ex_stack)@l
	cmplw	sp,r3
	blt	1f
	mr	sp,r3
    1:
	subi	sp,sp,EX_STACK_SIZE
	stw	r0,GPR0(sp)
	mfspr	r0,sprg2
	stw	r0,GPR1(sp)
	mfspr	r3,sprg0
	SAVE_GPRS
        mfspr   r3,sprg1
	stw	r3,LR(sp)
	mfspr	r3,srr2
	stw	r3,NIP(sp)
	mfctr	r3
	stw	r3,CTR(sp)
	mfspr	r3,sprg3
	stw	r3,CCR(sp)
	mfxer	r3
	stw	r3,XER(sp)
	mfspr	r3,srr3
	stw	r3,MSR(sp)
	li	r3,0
	stw	r3,MQ(sp)
	mflr	r3
	rlwinm	r3,r3,24,25,31
	stw	r3,TRAP(sp)
	mr	r4,sp
	bl	SYM_NAME(_bsp_exc_dispatch)

	/*
	 * need to call debug stub if exception not handled
	 * by application code.
	 */
	cmpwi	r3,0
	bne	1f
	lwz	r3,TRAP(sp)
	mr	r4,sp
	lis	r5,SYM_NAME(_bsp_dbg_vector)@h
	ori	r5,r5,SYM_NAME(_bsp_dbg_vector)@l
	lwz	r5,0(r5)
	mtlr	r5
	blrl
    1:
	lwz	r3,MSR(sp)
	mtspr   srr3,r3
	lwz	r3,NIP(sp)
	mtspr   srr2,r3
	lwz	r3,XER(sp)
	mtspr   xer,r3
	lwz	r3,CCR(sp)
	mtcr    r3
	lwz	r3,CTR(sp)
	mtctr   r3
	lwz	r3,LR(sp)
	mtlr	r3
	RESTORE_GPRS
	lwz	r0,GPR0(sp)
	lwz	sp,GPR1(sp)
	isync
	sync
	rfci
FUNC_END __ci_handler


/*
 * Programable interval timer interrupt uses srr2 and srr3.
 */
FUNC_START __pit_handler
	mtspr	sprg2,sp
	mfcr	r3
	mtspr	sprg3,r3
	lis	r3,SYM_NAME(_ex_stack)@h
	ori	r3,r3,SYM_NAME(_ex_stack)@l
	cmplw	sp,r3
	blt	1f
	mr	sp,r3
    1:
	subi	sp,sp,EX_STACK_SIZE
	stw	r0,GPR0(sp)
	mfspr	r0,sprg2
	stw	r0,GPR1(sp)
	mfspr	r3,sprg0
	SAVE_GPRS
        mfspr   r3,sprg1
	stw	r3,LR(sp)
	mfspr	r3,srr0
	stw	r3,NIP(sp)
	mfctr	r3
	stw	r3,CTR(sp)
	mfspr	r3,sprg3
	stw	r3,CCR(sp)
	mfxer	r3
	stw	r3,XER(sp)
	mfspr	r3,srr1
	stw	r3,MSR(sp)
	li	r3,0
	stw	r3,MQ(sp)
	li	r3,BSP_EXC_PIT
	stw	r4,TRAP(sp)
	mr	r4,sp
	bl	SYM_NAME(_bsp_exc_dispatch)

	/*
	 * need to call debug stub if exception not handled
	 * by application code.
	 */
	cmpwi	r3,0
	bne	1f
	lwz	r3,TRAP(sp)
	mr	r4,sp
	lis	r5,SYM_NAME(_bsp_dbg_vector)@h
	ori	r5,r5,SYM_NAME(_bsp_dbg_vector)@l
	lwz	r5,0(r5)
	mtlr	r5
	blrl
    1:
	lwz	r3,MSR(sp)
	mtspr   srr1,r3
	lwz	r3,NIP(sp)
	mtspr   srr0,r3
	lwz	r3,XER(sp)
	mtspr   xer,r3
	lwz	r3,CCR(sp)
	mtcr    r3
	lwz	r3,CTR(sp)
	mtctr   r3
	lwz	r3,LR(sp)
	mtlr	r3
	RESTORE_GPRS
	lwz	r0,GPR0(sp)
	lwz	sp,GPR1(sp)
	isync
	sync
	rfi
FUNC_END __pit_handler


/*
 * Fixed interval timer interrupt uses srr2 and srr3.
 */
FUNC_START __fit_handler
	mtspr	sprg2,sp
	mfcr	r3
	mtspr	sprg3,r3
	lis	r3,SYM_NAME(_ex_stack)@h
	ori	r3,r3,SYM_NAME(_ex_stack)@l
	cmplw	sp,r3
	blt	1f
	mr	sp,r3
    1:
	subi	sp,sp,EX_STACK_SIZE
	stw	r0,GPR0(sp)
	mfspr	r0,sprg2
	stw	r0,GPR1(sp)
	mfspr	r3,sprg0
	SAVE_GPRS
        mfspr   r3,sprg1
	stw	r3,LR(sp)
	mfspr	r3,srr0
	stw	r3,NIP(sp)
	mfctr	r3
	stw	r3,CTR(sp)
	mfspr	r3,sprg3
	stw	r3,CCR(sp)
	mfxer	r3
	stw	r3,XER(sp)
	mfspr	r3,srr1
	stw	r3,MSR(sp)
	li	r3,0
	stw	r3,MQ(sp)
	li	r3,BSP_EXC_FIT
	stw	r4,TRAP(sp)
	mr	r4,sp
	bl	SYM_NAME(_bsp_exc_dispatch)

	/*
	 * need to call debug stub if exception not handled
	 * by application code.
	 */
	cmpwi	r3,0
	bne	1f
	lwz	r3,TRAP(sp)
	mr	r4,sp
	lis	r5,SYM_NAME(_bsp_dbg_vector)@h
	ori	r5,r5,SYM_NAME(_bsp_dbg_vector)@l
	lwz	r5,0(r5)
	mtlr	r5
	blrl
    1:
	lwz	r3,MSR(sp)
	mtspr   srr1,r3
	lwz	r3,NIP(sp)
	mtspr   srr0,r3
	lwz	r3,XER(sp)
	mtspr   xer,r3
	lwz	r3,CCR(sp)
	mtcr    r3
	lwz	r3,CTR(sp)
	mtctr   r3
	lwz	r3,LR(sp)
	mtlr	r3
	RESTORE_GPRS
	lwz	r0,GPR0(sp)
	lwz	sp,GPR1(sp)
	isync
	sync
	rfi
FUNC_END __fit_handler

	
/*
 * Watchdog timer.
 */
FUNC_START __wdt_handler
	mtspr	sprg2,sp
	mfcr	r3
	mtspr	sprg3,r3
	lis	r3,SYM_NAME(_ex_stack)@h
	ori	r3,r3,SYM_NAME(_ex_stack)@l
	cmplw	sp,r3
	blt	1f
	mr	sp,r3
    1:
	subi	sp,sp,EX_STACK_SIZE
	stw	r0,GPR0(sp)
	mfspr	r0,sprg2
	stw	r0,GPR1(sp)
	mfspr	r3,sprg0
	SAVE_GPRS
        mfspr   r3,sprg1
	stw	r3,LR(sp)
	mfspr	r3,srr2
	stw	r3,NIP(sp)
	mfctr	r3
	stw	r3,CTR(sp)
	mfspr	r3,sprg3
	stw	r3,CCR(sp)
	mfxer	r3
	stw	r3,XER(sp)
	mfspr	r3,srr3
	stw	r3,MSR(sp)
	li	r3,0
	stw	r3,MQ(sp)
	li	r3,BSP_EXC_WATCHDOG
	stw	r3,TRAP(sp)
	mr	r4,sp
	bl	SYM_NAME(_bsp_exc_dispatch)

	/*
	 * need to call debug stub if exception not handled
	 * by application code.
	 */
	cmpwi	r3,0
	bne	1f
	lwz	r3,TRAP(sp)
	mr	r4,sp
	lis	r5,SYM_NAME(_bsp_dbg_vector)@h
	ori	r5,r5,SYM_NAME(_bsp_dbg_vector)@l
	lwz	r5,0(r5)
	mtlr	r5
	blrl
    1:
	lwz	r3,MSR(sp)
	mtspr   srr3,r3
	lwz	r3,NIP(sp)
	mtspr   srr2,r3
	lwz	r3,XER(sp)
	mtspr   xer,r3
	lwz	r3,CCR(sp)
	mtcr    r3
	lwz	r3,CTR(sp)
	mtctr   r3
	lwz	r3,LR(sp)
	mtlr	r3
	RESTORE_GPRS
	lwz	r0,GPR0(sp)
	lwz	sp,GPR1(sp)
	isync
	sync
	rfci
FUNC_END __wdt_handler

#endif /* __CPU_PPC4XX__ */	
		
	.data
	
	/*
	 *  Provide a stack for use by exception handler.
	 *  Note that this stack is in .bss, so it will be below
	 *  the normal user stack which is placed in high memory.
	 *  We use this fact to detect whether or not we need to
	 *  switch stacks before saving regs.
	 */
	.section ".bss","a"
	.p2align 4
	.rept	4096
	.long	0
	.endr
_ex_stack:	
	.long	0
