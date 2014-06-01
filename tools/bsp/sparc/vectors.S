/*
 * vectors.S -- SPARC BSP vectors.
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

#define LED_ADDR  0x02000003
	
.macro led val
	sethi	%hi(LED_ADDR),%l7
	set	\val,%l6
	stb	%l6,[%l7 + %lo(LED_ADDR)]
.endm

.macro delay
	set	10000000,%l7
    1:
	deccc	%l7
	bne	1b
	 nop
.endm
	

	.file "vectors.S"

.macro DO_TRAP vsr_num
	/*
	 * On entry:
	 *   %l1 = PC
	 *   %l2 = NPC
	 *
	 * Jumps to appropriate vector from bsp_vector_table.
	 */
	sethi	%hi(bsp_vsr_table),%l0
	ld	[%l0 + %lo(bsp_vsr_table) + (\vsr_num * 4)],%l0
	jmp	%l0
	 rd	%psr,%l0
.endm

#define TT_RESET     0
#define TT_TAGOVF   10
#define TT_INT1     17
#define TT_INT15    31
#define TT_CPDIS    36
#define TT_BREAK   129
#define TT_WINFLUSH 131
#define TT_SYSCALL 136
#define TT_DEBUG   255

	.section ".rom_vectors","ax"
#if defined(SINGLE_VECTOR_TRAP)
#error SINGLE_VECTOR_TRAP support not implemented
#else
T_reset:
	rd	%psr,%l0
	rd	%tbr,%l3
	b	SYM_NAME(start)
	 nop 

	DO_TRAP BSP_EXC_IACCESS
	DO_TRAP BSP_EXC_ILL
	DO_TRAP BSP_EXC_IPRIV
	DO_TRAP BSP_EXC_FPDIS
	DO_TRAP BSP_EXC_WINOVF
	DO_TRAP BSP_EXC_WINUND
	DO_TRAP BSP_EXC_ALIGN
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_DACCESS
	DO_TRAP BSP_EXC_TAGOVF

	.rept TT_INT1-TT_TAGOVF-1
	DO_TRAP BSP_EXC_TRAP
	.endr

	DO_TRAP BSP_EXC_INT1
	DO_TRAP BSP_EXC_INT2
	DO_TRAP BSP_EXC_INT3
	DO_TRAP BSP_EXC_INT4
	DO_TRAP BSP_EXC_INT5
	DO_TRAP BSP_EXC_INT6
	DO_TRAP BSP_EXC_INT7
	DO_TRAP BSP_EXC_INT8
	DO_TRAP BSP_EXC_INT9
	DO_TRAP BSP_EXC_INT10
	DO_TRAP BSP_EXC_INT11
	DO_TRAP BSP_EXC_INT12
	DO_TRAP BSP_EXC_INT13
	DO_TRAP BSP_EXC_INT14
	DO_TRAP BSP_EXC_INT15

	.rept TT_CPDIS-TT_INT15-1
	DO_TRAP BSP_EXC_TRAP
	.endr

	DO_TRAP BSP_EXC_CPDIS

	.rept TT_BREAK-TT_CPDIS-1
	DO_TRAP BSP_EXC_TRAP
	.endr
	
	DO_TRAP BSP_EXC_BREAK
	
	.rept TT_WINFLUSH-TT_BREAK-1
	DO_TRAP BSP_EXC_TRAP
	.endr
	
	DO_TRAP BSP_EXC_WINFLUSH
	
	.rept TT_SYSCALL-TT_WINFLUSH-1
	DO_TRAP BSP_EXC_TRAP
	.endr
	
	DO_TRAP BSP_EXC_SYSCALL
	
	.rept TT_DEBUG-TT_SYSCALL-1
	DO_TRAP BSP_EXC_TRAP
	.endr

	DO_TRAP BSP_EXC_DEBUG
#endif

	/*
	 * Default window overflow handler.
	 * On entry:
	 *   %l1 = PC
	 *   %l2 = NPC
	 *
	 * This routine runs with traps disabled, and care must be
	 * taken not to touch the condition codes as PSR is never
	 * restored.
	 */
FUNC_START _bsp_wovf_handler
	mov	%wim,%l0
	mov	%g1,%l3			/* save %g1 */
	srl	%l0,1,%g1
	sll	%l0,CPU_WINSIZE-1,%l0
	or	%l0,%g1,%g1

	save	%g0,%g0,%g0		/* Slip into next window */
	mov	%g1,%wim		/* Install the new wim   */
	 nop
	 nop
	 nop

	std	%l0,[%sp +  0 * 4]	/* save L & I registers  */
	std	%l2,[%sp +  2 * 4]
	std	%l4,[%sp +  4 * 4]
	std	%l6,[%sp +  6 * 4]
	std	%i0,[%sp +  8 * 4]
	std	%i2,[%sp + 10 * 4]
	std	%i4,[%sp + 12 * 4]
	std	%i6,[%sp + 14 * 4]

	restore				/* Go back to trap window. */
	mov	%l3,%g1			/* Restore %g1 */

	jmpl	%l1,%g0
	 rett	%l2
FUNC_END _bsp_wovf_handler

	/*
	 * Default window underflow handler.
	 * On entry:
	 *   %l0 = PSR
	 *   %l1 = PC
	 *   %l2 = NPC
	 *
	 * This routine runs with traps disabled, and care must be
	 * taken not to touch the condition codes as PSR is never
	 * restored.
	 */
FUNC_START _bsp_wund_handler
	mov	%wim,%l0
	sll	%l0,1,%l3		/* Rotate wim left */
	srl	%l0,CPU_WINSIZE-1,%l0
	or	%l0,%l3,%l0

	mov	%l0,%wim		/* Install the new wim */
	 nop
	 nop
	 nop

	restore				/* Users window       */
	restore				/* His callers window */

	ldd	[%sp +  0 * 4],%l0	/* restore L & I registers */
	ldd	[%sp +  2 * 4],%l2
	ldd	[%sp +  4 * 4],%l4
	ldd	[%sp +  6 * 4],%l6
	ldd	[%sp +  8 * 4],%i0
	ldd	[%sp + 10 * 4],%i2
	ldd	[%sp + 12 * 4],%i4
	ldd	[%sp + 14 * 4],%i6

	save	%g0,%g0,%g0		/* Back to trap window */
	save	%g0,%g0,%g0

	jmpl	%l1,%g0
	 rett	%l2
FUNC_END _bsp_wund_handler


	/*
	 * Default window flush handler.
	 * On entry:
	 *   %l1 = PC
	 *   %l2 = NPC
	 *
	 * This routine runs with traps disabled, and care must be
	 * taken not to touch the condition codes as PSR is never
	 * restored.
	 */
FUNC_START _bsp_wflush_handler
	/* enable traps */
	or	%l0,0x20,%l0
	wr	%l0,%psr
	 nop
	 nop
	 nop
	
	set	temp_sptr,%l5
	st	%l2,[%l5]
	
	save %sp, -64, %sp
	save %sp, -64, %sp
	save %sp, -64, %sp
	save %sp, -64, %sp
	save %sp, -64, %sp
	save %sp, -64, %sp
	save %sp, -64, %sp
	save %sp, -64, %sp
	restore
	restore
	restore
	restore
	restore
	restore
	restore
	restore

	restore			/* Make sure we have a valid window */
	save	%g0,%g0,%g0

	set	temp_sptr,%l2	/* Get our return address back */
	ld	[%l2],%l2

	/* restore PSR */
	rd	%psr,%l0
	andn	%l0,0x20,%l0
	wr	%l0,%psr
	 nop
	 nop
	 nop
	
	/* skip trap insn */	
	jmpl	%l2,%g0
	 rett	%l2+4
FUNC_END _bsp_wflush_handler


FUNC_START _default_handler
	mov	%tbr,%g1
	srl	%g1,4,%g1
	not	%g1,%g1
    9:
	sethi	%hi(LED_ADDR),%l7
	stb	%g1,[%l7 + %lo(LED_ADDR)]
	delay
	led	0xff
	delay
	ba,a	9b
	 nop
FUNC_END _default_handler

	.section ".ram_vectors","ax"
	.globl bsp_vsr_table
bsp_vsr_table:
	.word _default_handler		/* insn access */
	.word _default_handler		/* illegal insn */
	.word _default_handler		/* insn privilege */
	.word _default_handler		/* FPU disabled */
	.word _bsp_wovf_handler		/* window overflow */
	.word _bsp_wund_handler		/* window underflow */
	.word _default_handler		/* alignment */
	.word _default_handler		/* data access */
	.word _default_handler		/* tag overflow */
	.word _default_handler		/* int 1 */
	.word _default_handler		/* int 2 */
	.word _default_handler		/* int 3 */
	.word _default_handler		/* int 4 */
	.word _default_handler		/* int 5 */
	.word _default_handler		/* int 6 */
	.word _default_handler		/* int 7 */
	.word _default_handler		/* int 8 */
	.word _default_handler		/* int 9 */
	.word _default_handler		/* int 10 */
	.word _default_handler		/* int 11 */
	.word _default_handler		/* int 12 */
	.word _default_handler		/* int 13 */
	.word _default_handler		/* int 14 */
	.word _default_handler		/* int 15 */
	.word _default_handler		/* Coprocessor disabled */
	.word _default_handler		/* breakpoint */
	.word _bsp_wflush_handler	/* window flush */
	.word _default_handler		/* syscall */
	.word _default_handler		/* debug unit */
	.word _default_handler		/* other traps */
	.word 0				/* MT debugging hook */
	.word 0				/* stub entry hook   */
	.word 0				/* bsp shared data   */

temp_sptr:	
	.word	0

	.globl bsp_vsr_xlate
bsp_vsr_xlate:
	.byte 0			/* place_holder for reset */
	.byte BSP_EXC_IACCESS
	.byte BSP_EXC_ILL
	.byte BSP_EXC_IPRIV
	.byte BSP_EXC_FPDIS
	.byte BSP_EXC_WINOVF
	.byte BSP_EXC_WINUND
	.byte BSP_EXC_ALIGN
	.byte BSP_EXC_TRAP
	.byte BSP_EXC_DACCESS
	.byte BSP_EXC_TAGOVF
	.rept TT_INT1-TT_TAGOVF-1
	.byte BSP_EXC_TRAP
	.endr
	.byte BSP_EXC_INT1
	.byte BSP_EXC_INT2
	.byte BSP_EXC_INT3
	.byte BSP_EXC_INT4
	.byte BSP_EXC_INT5
	.byte BSP_EXC_INT6
	.byte BSP_EXC_INT7
	.byte BSP_EXC_INT8
	.byte BSP_EXC_INT9
	.byte BSP_EXC_INT10
	.byte BSP_EXC_INT11
	.byte BSP_EXC_INT12
	.byte BSP_EXC_INT13
	.byte BSP_EXC_INT14
	.byte BSP_EXC_INT15
	.rept TT_CPDIS-TT_INT15-1
	.byte BSP_EXC_TRAP
	.endr
	.byte BSP_EXC_CPDIS
	.rept TT_BREAK-TT_CPDIS-1
	.byte BSP_EXC_TRAP
	.endr
	.byte BSP_EXC_BREAK
	.rept TT_WINFLUSH-TT_BREAK-1
	.byte BSP_EXC_TRAP
	.endr
	.byte BSP_EXC_WINFLUSH
	.rept TT_SYSCALL-TT_WINFLUSH-1
	.byte BSP_EXC_TRAP
	.endr
	.byte BSP_EXC_SYSCALL
	.rept TT_DEBUG-TT_SYSCALL-1
	.byte BSP_EXC_TRAP
	.endr
	.byte BSP_EXC_DEBUG



