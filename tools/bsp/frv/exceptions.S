/*
 * exceptions.S -- FR-V default exception handling.
 *
 * Copyright (c) 1999, 2000 Cygnus Support
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

	.file "exceptions.S"
	.text

	/*
	 * This function is called when an NMI occurs.
	 * We have to clear the interrupt before re-enabling traps.
	 * This is very platform specific.
	 */
FUNC_START _bsp_nmi_handler
	nop
FUNC_END _bsp_nmi_handler


	/*
	 * On entry:
	 *   sp    -->
	 *   sp-4  --> gr4
	 *   sp-8  --> gr5
	 *   sp-12 --> ccr
	 *   gr4   --> unknown
	 *   gr5   --> vsr# * 4
	 */
FUNC_START _bsp_default_handler
	/* save vsr# */
	srli	gr5,2,gr5
	sti	gr5,@(sp,-16)

	movsg	lr,gr5
	call	1f
    1:
	movsg	lr,gr4
	movgs	gr5,lr
	sethi	#gprelhi(1b),gr5
	setlo	#gprello(1b),gr5
	sub	gr4,gr5,gr4

	/* switch to exception stack */
	sethi	#gprelhi(SYM_NAME(_ex_stack)),gr5
	setlo	#gprello(SYM_NAME(_ex_stack)),gr5
	add	gr4,gr5,gr4

	or	sp,gr0,gr5
	subcc	gr4,sp,gr0,icc0
	bge	icc0,0,1f
	or	gr4,gr0,sp	
    1:
	setlos	EX_STACK_SIZE,gr4
	sub	sp,gr4,sp
	
	/* save original stack */
	sti	gr5,@(sp,FR_SP)

	/* restore temp saved regs */
	ldi	@(gr5,-16),gr4
	sti	gr4,@(sp,FR_VSRNUM)
	ldi	@(gr5,-12),gr4
	sti	gr4,@(sp,FR_CCR)
	ldi	@(gr5,-4),gr4
	ldi	@(gr5,-8),gr5
	
	/* save regs */
	save_all_gr
	save_all_fr
	save_sprs
	
	movsg	psr,gr6
	sti	gr6,@(sp,FR_PSR)

	/*
	 * break/not break handling
	 */
	ldi	@(sp,FR_VSRNUM),gr4
	subicc	gr4,BSP_EXC_BREAK,gr0,icc0
	ldi	@(sp,FR_PCSR),gr4
	bne	icc0,0,1f
	/* break exception */
	setlos	0xff,gr5
	bra	2f
    1:
	/* not break exception */
	movsg	tbr,gr5
	srli	gr5,4,gr5
	andi	gr5,0xff,gr5
    2:
	sti	gr4,@(sp,FR_PC)
	sti	gr5,@(sp,FR_TRAP)

	/*
	 * Disable interrupts, enable traps.
	 */
	ori	gr6,PSR_PIL_MASK,gr4
	movgs	gr4,psr
	ori	gr4,PSR_ET,gr4
	movgs	gr4,psr
	
	/*
	 * If we were in user mode and the PSR.ESR bit is set,
	 * then we need to get the user gr4-7 from the shadow
	 * registers.
	 */
	srli	gr4,11,gr4
	andicc	gr4,1,gr0,icc0
	beq	icc0,0,1f
	/* PSR.ESR is set */
	andicc	gr6,2,gr0,icc0
	bne	icc0,0,1f
	/* we were in user mode */
	movsg	sr0,gr4
	sti	gr4,@(sp,FR_GR0+(4*4))
	movsg	sr1,gr5
	sti	gr5,@(sp,FR_GR0+(5*4))
	movsg	sr2,gr6
	sti	gr6,@(sp,FR_GR0+(6*4))
	movsg	sr3,gr7
	sti	gr7,@(sp,FR_GR0+(7*4))
    1:

	/* load up small data pointer */
	call	1f
    1:
	movsg	lr,gr16
	sethi	#gprelhi(1b),gr5
	setlo	#gprello(1b),gr5
	sub	gr16,gr5,gr16
	
	/*
	 * Dispatch to BSP exception handler.
	 */
	ldi	@(sp,FR_VSRNUM),gr8
	or	sp,gr0,gr9
	call	SYM_NAME(_bsp_exc_dispatch)

	/* setup icc3 for check to see if we need debug agent */
	orcc	gr8,gr0,gr0,icc3

	/* restore regs */
	restore_sprs
	restore_all_fr	
	restore_all_gr

	/*
	 * Restore sr0-sr3 if necessary
	 */
	ldi	@(sp,FR_PSR),gr14
	movgs	gr14,psr
	andicc	gr14,2,gr0,icc1
	bne	icc1,0,3f
	/* returning to user mode */
    2:
	srli	gr14,11,gr14
	andicc	gr14,1,gr0,icc1
	beq	icc1,0,3f
	/* PSR.ESR is set */
	movgs	gr4,sr0
	movgs	gr5,sr1
	movgs	gr6,sr2
	movgs	gr7,sr3
    3:
	bne	icc3,0,SYM_NAME(_normal_return)
	/*
	 * Need to call debug agent with:
	 *   
	 *   sp    -->
	 *   sp-4  --> saved gr4
	 *   sp-8  --> saved gr5
	 *   gr4   --> unknown
	 *   gr5   --> exception#
	 *   
	 */
	ldi	@(sp,FR_SP),gr14
	sti	gr4,@(gr14,-4)
	sti	gr5,@(gr14,-8)
	ldi	@(sp,FR_TRAP),gr5
	ldi	@(sp,FR_CCR),gr14
	movgs	gr14,ccr

	movsg	lr,gr4
	call	1f
    1:
	movsg	lr,gr14
	movgs	gr4,lr
	sethi	#gprelhi(1b),gr4
	setlo	#gprello(1b),gr4
	sub	gr14,gr4,gr14
	sethi	#gprelhi(SYM_NAME(bsp_vsr_table)),gr4
	setlo	#gprello(SYM_NAME(bsp_vsr_table)),gr4
	add	gr4,gr14,gr4
	lddi	@(sp,FR_GR0+(14*4)),gr14
	ldi	@(sp,FR_SP),sp
	ldi	@(gr4,BSP_VEC_STUB_ENTRY*4),gr4
	jmpl	@(gr4,gr0)

SYM_NAME(_normal_return):
	ldi	@(sp,FR_CCR),gr14
	movgs	gr14,ccr
	lddi	@(sp,FR_GR0+(14*4)),gr14
	ldi	@(sp,FR_SP),sp
	rett	0
FUNC_END _bsp_default_handler

	/*
	 *  Provide a stack for use by exceptions and interrupts.
	 *  Note that this stack is in .bss, so it will be below
	 *  the normal user stack which is placed in high memory.
	 */
	.section ".bss","a"
	.p2align 4
	.rept	2048
	.long	0
	.endr
	.globl SYM_NAME(_ex_stack)
SYM_NAME(_ex_stack):	
