/*
 * stublow.S -- FR-V low-level stub entry.
 *
 * Copyright (c) 1999, 2000 Red Hat, Inc.
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
#include "syscall.h"

	.file "stublow.S"
	.text
	.extern SYM_NAME(_ex_stack)
	.extern SYM_NAME(__switch_trapnum)

	/*
	 * On entry:
	 *   sp    -->
	 *   sp-4  --> saved gr4
	 *   sp-8  --> saved gr5
	 *   gr4   --> unknown
	 *   gr5   --> exception#
	 */
FUNC_START _bsp_stub_low
	/* save ccr */
	movsg	ccr,gr4
	sti	gr4,@(sp,-12)
	sti	gr5,@(sp,-16)

	/* pic setup */
	movsg	lr,gr5
	call	1f
    1:
	movsg	lr,gr4
	movgs	gr5,lr
	sethi	#gprelhi(1b),gr5
	setlo	#gprello(1b),gr5
	sub	gr4,gr5,gr4

	/* now test for break trap */
	ldi	@(sp,-16),gr5
	subicc	gr5,0xff,gr0,icc0
	beq	icc0,0,_do_break

	/* need to switch to debug state before calling the debug agent */
	
	/* save trap number */
	sethi	%gprelhi(SYM_NAME(__switch_trapnum)),gr5
	setlo	%gprello(SYM_NAME(__switch_trapnum)),gr5
	add	gr4,gr5,gr4
	ldi	@(sp,-16),gr5
	st	gr5,@(gr4,gr0)

	/* restore regs */
	ldi	@(sp,-12),gr4
	movgs	gr4,ccr
	ldi	@(sp,-4),gr4
	ldi	@(sp,-8),gr5
	
	.globl SYM_NAME(__break_switch)
    SYM_NAME(__break_switch):
	break	/* cause a break trap */

	/* Never reached. The break handler will setup a return to the
	   spot of the original exception. */
_do_break:
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
	sti	gr4,@(sp,FR_TRAP)
	ldi	@(gr5,-12),gr4
	sti	gr4,@(sp,FR_CCR)
	ldi	@(gr5,-4),gr4
	ldi	@(gr5,-8),gr5
	
	/* save regs */
	save_all_gr
	save_all_fr
	save_sprs
	
	movsg	dbar0,gr4
	sti	gr4,@(sp,FR_DBAR0)
	movsg	dbar1,gr4
	sti	gr4,@(sp,FR_DBAR1)
	movsg	dbar2,gr4
	sti	gr4,@(sp,FR_DBAR2)
	movsg	dbar3,gr4
	sti	gr4,@(sp,FR_DBAR3)

	movsg	psr,gr6
	sti	gr6,@(sp,FR_PSR)
	ldi	@(sp,FR_BPCSR),gr4
	sti	gr4,@(sp,FR_PC)

	/* enable exception breaks */
	movsg	dcr,gr4
	sethi	0x4000,gr5
	setlo	0,gr5
	or	gr4,gr5,gr4
	movgs	gr4,dcr

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
	or	sp,gr0,gr8
	call	SYM_NAME(_bsp_break_handler)

	/* restore regs */
	restore_sprs
	restore_all_fr	
	restore_all_gr

	ldi	@(sp,FR_PSR),gr14
	movgs	gr14,psr

	/*
	 * Restore sr0-sr3 if necessary
	 */
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
	ldi	@(sp,FR_CCR),gr14
	movgs	gr14,ccr
	lddi	@(sp,FR_GR0+(14*4)),gr14
	ldi	@(sp,FR_SP),sp
	rett	1
FUNC_END _bsp_stub_low

