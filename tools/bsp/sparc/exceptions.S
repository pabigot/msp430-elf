/*
 * exceptions.S -- SPARC default exception handling.
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

	.text

	/*
	 * This function is called when an NMI occurs.
	 * We have to clear the interrupt before re-enabling traps.
	 * This is very platform specific.
	 *
	 * On entry:
	 *    %l0 - psr
	 *    %l1 - pc
	 *    %l2 - npc
	 */
FUNC_START _bsp_nmi_handler
#define IRQCLR_REGADDR    0x1000000C
#define IRQLATCH_REGADDR  0x10000014
	sethi	%hi(0x80000000),%l3	/* IRQ 15 bit */
	set	IRQCLR_REGADDR,%l4
	sta	%l3,[%l4] 4		/* clear the irq */
	set	0x100000,%l3		/* latch clear bit */
	set	IRQLATCH_REGADDR,%l4
	sta	%l3,[%l4] 4		/* clear the latch */
	ba,a	_bsp_default_handler
	 nop
FUNC_END _bsp_nmi_handler


FUNC_START _bsp_default_handler
	/*
	 * This function is called when any SPARC trap (except window overflow,
	 * underflow, or flush) occurs. It makes sure that the invalid register
	 * window is still available before jumping into C code.
	 *
	 * On entry:
	 *    %l0 - psr
	 *    %l1 - pc
	 *    %l2 - npc
	 *
	 * Register usage throughout the routine is as follows:
	 *
	 *    %l0 - psr
	 *    %l1 - pc
	 *    %l2 - npc
	 *    %l3 - wim
	 *    %l4 - scratch and y reg
	 *    %l5 - tbr
	 *    %l6 - unused
	 *    %l7 - flag to vector to debug stub
	 */

	/* enable traps, disable interrupts */
	or	%l0,0xf20,%l3
	wr	%l3,%psr
	 nop
	 nop
	 nop

	/* make sure we're not using the invalid window */
	mov	%wim,%l3
	srl	%l3,%l0,%l4	/* wim >> cwp */
	cmp	%l4, 1
	bne	window_fine	/* Branch if not in the invalid window */
	 mov	%tbr,%l5
	
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
	ldub	[%o0 + %o1],%o0		/* VSR num is first arg */
	call	SYM_NAME(_bsp_exc_dispatch)
	 add	%sp,FR_G0,%o1		/* pointer to regs is second arg */
	mov	%o0,%l7			/* save return value */

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
	andn	%l1,%l4,%l1
	or	%l1,%l5,%l0
	
	/* Oh, and don't mess with the WIM either. Why? We invalidated the
	   saved value when we flushed the registers to the stack. */
	
	/* see if we need to invoke the debug stub */
	cmp	%l7,%g0
	bne,a	1f
	 mov	%l0,%psr

	/*
	 * Need to call stub with:
	 *   %l0 - %psr
	 *   %l1 - PC
	 *   %l2 - NPC
	 *   %l3 - %tbr
	 */
	mov	%l2,%l1
	mov	%l3,%l2
	ld	[%sp + FR_TBR],%l3
	sethi	%hi(bsp_vsr_table),%l4
	ld	[%l4 + BSP_VEC_STUB_ENTRY*4],%l4
	jmpl	%l4,%g0
    1:
	 nop
	nop
	nop
	jmpl	%l2,%g0
	 rett	%l3
FUNC_END _bsp_default_handler

