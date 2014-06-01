/*
 * stublow.S -- SPARC stub entry code.
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

	.file "stublow.S"

	.text

FUNC_START _bsp_stub_entry
	/*
	 * This function is vectored to when the debug stub needs to
	 * gain control of the system.
	 *
	 * On entry:
	 *    %l0 - psr
	 *    %l1 - pc
	 *    %l2 - npc
	 *    %l3 - tbr
	 */
	
	/* enable traps, disable interrupts */
	or	%l0,0xf20,%l4
	wr	%l4,%psr
	 nop
	 nop
	 nop
	mov	%l3,%l5		/* mov %tbr */

	/* make sure we're not using the invalid window */
	mov	%wim,%l3
	srl	%l3,%l0,%l4	/* wim >> cwp */
	cmp	%l4, 1
	bne	window_fine	/* Branch if not in the invalid window */
	 nop

	/*
	 * Handle window overflow.
	 */
	mov	%g1,%l4		/* Save g1, we use it to hold the wim */
	srl	%l3,1,%g1	/* Rotate wim right */
	tst	%g1
	bg	good_wim	/* Branch if new wim is non-zero */
	 nop

	/*
	 * At this point, we need to bring a 1 into the high order bit of the
	 * wim. Since we don't want to make any assumptions about the number
	 * of register windows, we figure it out dynamically so as to setup
	 * the wim correctly.
	 */
	not	%g1		/* Fill g1 with ones */
	mov	%g1,%wim	/* Fill the wim with ones */
	 nop
	 nop
	 nop
	mov	%wim,%g1	/* Read back the wim */
	inc	%g1		/* Now g1 has 1 just to left of wim */
	srl	%g1,1,%g1	/* Now put 1 at top of wim */
	mov	%g0,%wim	/* Clear wim so that next save won't trap */
	 nop
	 nop
	 nop

  good_wim:
	save	%g0,%g0,%g0	/* Slip into next window */
	mov	%g1,%wim	/* Install the new wim */
	 nop
	 nop
	 nop
	std	%l0,[%sp +  0 * 4]  /* save L & I registers */
	std	%l2,[%sp +  2 * 4]
	std	%l4,[%sp +  4 * 4]
	std	%l6,[%sp +  6 * 4]
	std	%i0,[%sp +  8 * 4]
	std	%i2,[%sp + 10 * 4]
	std	%i4,[%sp + 12 * 4]
	std	%i6,[%sp + 14 * 4]
	restore			/* Go back to trap window. */
	mov	%l4, %g1	/* Restore %g1 */

  window_fine:
	/*
	 * Switch to exception stack if we aren't already using it.
	 */
	mov	%fp,%sp		/* make sure we have a usable %sp */
	set	__ex_stack,%l4
	cmp	%sp,%l4
	ble	1f
	 nop
	mov	%l4,%sp
    1:
	/*
	 * Make room for an exception frame.
	 */
	sub	%sp,EX_STACK_SIZE,%sp

	/* guarantee dword alignment */
	set	7,%l6
	not	%l6
	and	%sp,%l6,%sp
	
	std	%g0,[%sp + FR_G0]
	std	%g2,[%sp + FR_G2]
	std	%g4,[%sp + FR_G4]
	std	%g6,[%sp + FR_G6]
	std	%i0,[%sp + FR_O0]
	std	%i2,[%sp + FR_O2]
	std	%i4,[%sp + FR_O4]
	std	%i6,[%sp + FR_O6]
	mov	%y,%l4
	st	%l4,[%sp + FR_Y]
	st	%l0,[%sp + FR_PSR]
	st	%l3,[%sp + FR_WIM]
	st	%l5,[%sp + FR_TBR]
	st	%l1,[%sp + FR_PC]
	st	%l2,[%sp + FR_NPC]
	set	0x1000,%l1
	btst	%l1,%l0		/* FP enabled? */
	be	no_fpstore
	 nop
	/* 
	 * Must save fsr first, to flush the FQ.  This may cause a deferred
	 * fp trap, so traps must be enabled to allow the trap handler to
	 * clean things up.
	 */
	st	%fsr,[%sp + FR_FPSR]
	std	%f0,[%sp + FR_FREG0]
	std	%f2,[%sp + FR_FREG2]
	std	%f4,[%sp + FR_FREG4]
	std	%f6,[%sp + FR_FREG6]
	std	%f8,[%sp + FR_FREG8]
	std	%f10,[%sp + FR_FREG10]
	std	%f12,[%sp + FR_FREG12]
	std	%f14,[%sp + FR_FREG14]
	std	%f16,[%sp + FR_FREG16]
	std	%f18,[%sp + FR_FREG18]
	std	%f20,[%sp + FR_FREG20]
	std	%f22,[%sp + FR_FREG22]
	std	%f24,[%sp + FR_FREG24]
	std	%f26,[%sp + FR_FREG26]
	std	%f28,[%sp + FR_FREG28]
	std	%f30,[%sp + FR_FREG30]
  no_fpstore:
	set	bsp_vsr_xlate,%o0	/* address of TT->VSR xlate table */
	srl	%l5,4,%o1		/* shift and mask TT field of TBR */
	and	%o1,255,%o1
	ldub	[%o0 + %o1],%l5		/* VSR num is first arg */
	
	/*
	 * Flush all registers windows and copy Iregs and Lregs from stack
	 * to our frame.
	 */
	save	%sp,-64,%sp
	save	%sp,-64,%sp
	save	%sp,-64,%sp
	save	%sp,-64,%sp
	save	%sp,-64,%sp
	save	%sp,-64,%sp
	save	%sp,-64,%sp
	save	%sp,-64,%sp
	restore
	restore
	restore
	restore
	restore
	restore
	restore
	restore

	/* copy registers from user stack to exception stack */
	ld	[%sp + FR_SP],%g1
	add	%sp,FR_L0,%g2
	mov	15*4,%g3
    1:
	ld	[%g1+%g3],%l0
	st	%l0,[%g2+%g3]
	cmp	%g3,%g0
	bg	1b
	 add	%g3,-4,%g3

	/* save sparclite debug unit regs */
	sethi	%hi(0xff00),%l0
	or	%l0,%lo(0xff00),%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DIA1]
	add	%l0,4,%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DIA2]
	add	%l0,4,%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DDA1]
	add	%l0,4,%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DDA2]
	add	%l0,4,%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DDV1]
	add	%l0,4,%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DDV2]
	add	%l0,4,%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DCR]
	add	%l0,4,%l0
	lda	[%l0] 1,%l6
	st 	%l6,[%sp + FR_DSR]

	rd	%asr17,%l6
	st	%l6,[%sp + FR_ASR17]
		
	sethi	%hi(SYM_NAME(_bsp_dbg_vector)),%l0
	ld	[%l0 + %lo(SYM_NAME(_bsp_dbg_vector))],%l0
	
	mov	%l5,%o0
	call	%l0
	 add	%sp,FR_G0,%o1		/* pointer to regs is second arg */
	
	/* restore sparclite debug unit regs */
	sethi	%hi(0xff00),%l0
	or	%l0,%lo(0xff00),%l0
	ld 	[%sp + FR_DIA1],%l6
	sta	%l6,[%l0] 1
	add	%l0,4,%l0
	ld 	[%sp + FR_DIA2],%l6
	sta	%l6,[%l0] 1
	add	%l0,4,%l0
	ld 	[%sp + FR_DDA1],%l6
	sta	%l6,[%l0] 1
	add	%l0,4,%l0
	ld 	[%sp + FR_DDA2],%l6
	sta	%l6,[%l0] 1
	add	%l0,4,%l0
	ld 	[%sp + FR_DDV1],%l6
	sta	%l6,[%l0] 1
	add	%l0,4,%l0
	ld 	[%sp + FR_DDV2],%l6
	sta	%l6,[%l0] 1
	add	%l0,4,%l0
	ld 	[%sp + FR_DCR],%l6
	sta	%l6,[%l0] 1
	add	%l0,4,%l0
	ld 	[%sp + FR_DSR],%l6
	sta	%l6,[%l0] 1
	
	ld	[%sp + FR_ASR17],%l6
	wr	%l6,%asr17
		
	/* restore %tbr */
	ld 	[%sp + FR_TBR],%l6
	andn	%l6,0xfff,%l6
	wr	%l6,%tbr
		
	/* Reload all of the registers that aren't on the stack */
  done:
	ld	[%sp + FR_G1],%g1
	ldd	[%sp + FR_G2],%g2
	ldd	[%sp + FR_G4],%g4
	ldd	[%sp + FR_G6],%g6
	ldd	[%sp + FR_O0],%i0
	ldd	[%sp + FR_O2],%i2
	ldd	[%sp + FR_O4],%i4
	ldd	[%sp + FR_O6],%i6
	ldd	[%sp + FR_Y],%l0  /* Y & PSR  */
	ldd	[%sp + FR_PC],%l2 /* PC & NPC */
	set	0x1000,%l5
	btst	%l5,%l1
	be	no_fpreload
	 nop
	ldd	[%sp + FR_FREG0],%f0
	ldd	[%sp + FR_FREG2],%f2
	ldd	[%sp + FR_FREG4],%f4
	ldd	[%sp + FR_FREG6],%f6
	ldd	[%sp + FR_FREG8],%f8
	ldd	[%sp + FR_FREG10],%f10
	ldd	[%sp + FR_FREG12],%f12
	ldd	[%sp + FR_FREG14],%f14
	ldd	[%sp + FR_FREG16],%f16
	ldd	[%sp + FR_FREG18],%f18
	ldd	[%sp + FR_FREG20],%f20
	ldd	[%sp + FR_FREG22],%f22
	ldd	[%sp + FR_FREG24],%f24
	ldd	[%sp + FR_FREG26],%f26
	ldd	[%sp + FR_FREG28],%f28
	ldd	[%sp + FR_FREG30],%f30
	ld	[%sp + FR_FPSR],%fsr
  no_fpreload:
	restore			/* Ensure that previous window is valid */
	save	%g0,%g0,%g0	/* by causing a window_underflow trap   */
	mov	%l0,%y
	
	/* Preserve the CWP and supervisor bit, but otherwise...*/
	set	0x9f,%l4
	mov	%psr,%l5
	and	%l5,%l4,%l5
	not	%l4
	and	%l4,%l1,%l1
	or	%l1,%l5,%l1
	/* Oh, and don't mess with the WIM either. Why? We invalidated the
	   saved value when we flushed the registers to the stack. */
	mov	%l1,%psr
	 nop
	 nop
	 nop
	
	jmpl	%l2,%g0
	 rett	%l3
FUNC_END _bsp_stub_entry


	/*
	 *  Provide a stack for use exceptions and interrupts.
	 *  Note that this stack is in .bss, so it will be below
	 *  the normal user stack which is placed in high memory.
	 */
	.section ".bss","a"
	.p2align 4
	.rept	2048
	.long	0
	.endr
	.globl __ex_stack
  __ex_stack:	

