/*
 * vectors.S -- FR-V BSP vectors.
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
#include "fr500.h"

	.macro DELAY
	sethi	#3,gr8
	setlo	#0,gr8
     1:
	subicc	gr8,1,gr8,icc0
	bne	icc0,0x0,1b
	.endm

	.macro LED val
	sethi	#0x1200,gr4
	setlo	#0,gr4
	setlos	#\val,gr8
	sti	gr8,@(gr4,0)
	sti	gr8,@(gr4,8)
	.endm
	
	
	.file "vectors.S"

	.macro DO_TRAP vsr_num
	/*
	 * Jumps to a trampoline. I wish the ISA would allow us
	 * to load and jmp to the vsr from the table in 4 insns.
	 * It doesn't.
	 * Also, a valid stack is assumed. We are sunk if the
	 * sp is unusable.
	 */
	sti	gr4,@(sp,-4)
	sti	gr5,@(sp,-8)
	setlos	\vsr_num * 4, gr5
	bra	SYM_NAME(_exc_trampoline)
	.endm

	.section ".rom_vectors","ax"
T_reset:
	call	SYM_NAME(start)			/* 0 */
	nop
	nop
	nop

	DO_TRAP BSP_EXC_IACCESS_MMU
	DO_TRAP BSP_EXC_IACCESS_ERROR
	DO_TRAP BSP_EXC_IACCESS_EXC
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_TRAP			/* 5 */
	DO_TRAP BSP_EXC_IPRIVILEGE
	DO_TRAP BSP_EXC_ILLEGAL
	DO_TRAP BSP_EXC_REG_EXCEPTION
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_FP_DISABLED		/* 10 */
	DO_TRAP BSP_EXC_MP_DISABLED
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_FP_EXC
	DO_TRAP BSP_EXC_MP_EXC
	DO_TRAP BSP_EXC_TRAP			/* 15 */
	DO_TRAP BSP_EXC_MEM_ALIGN
	DO_TRAP BSP_EXC_DACCESS_ERROR
	DO_TRAP BSP_EXC_DACCESS_MMU
	DO_TRAP BSP_EXC_DACCESS_EXC
	Do_TRAP BSP_EXC_DSTORE_ERROR		/* 20 */
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_DIVIDE
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_COMMIT			/* 25 */
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_TRAP			/* 30 */
	DO_TRAP BSP_EXC_TRAP
	DO_TRAP BSP_EXC_COMPOUND
	DO_TRAP BSP_EXC_INT1
	DO_TRAP BSP_EXC_INT2
	DO_TRAP BSP_EXC_INT3			/* 35 */
	DO_TRAP BSP_EXC_INT4
	DO_TRAP BSP_EXC_INT5
	DO_TRAP BSP_EXC_INT6
	DO_TRAP BSP_EXC_INT7
	DO_TRAP BSP_EXC_INT8			/* 40 */
	DO_TRAP BSP_EXC_INT9
	DO_TRAP BSP_EXC_INT10
	DO_TRAP BSP_EXC_INT11
	DO_TRAP BSP_EXC_INT12
	DO_TRAP BSP_EXC_INT13			/* 45 */
	DO_TRAP BSP_EXC_INT14
	DO_TRAP BSP_EXC_INT15
	
	.rept TT_SYSCALL-TT_INT15-1
	DO_TRAP BSP_EXC_TRAP
	.endr
	
	DO_TRAP BSP_EXC_SYSCALL
	
	.rept TT_BREAK-TT_SYSCALL-1
	DO_TRAP BSP_EXC_TRAP
	.endr

	/* BREAK exception */	
	sti	gr4,@(sp,-4)
	sti	gr5,@(sp,-8)
	setlos	0xff,gr5

	/* temp save gr6 while we construct pic safe address */
	sti	gr6,@(sp,-12)
	movsg	lr,gr4
	call	1f
    1:
	movsg	lr,gr6
	movgs	gr4,lr
	sethi	%gprelhi(1b),gr4
	setlo	%gprello(1b),gr4
	sub	gr6,gr4,gr6
	sethi	%gprelhi(SYM_NAME(bsp_vsr_table)),gr4
	setlo	%gprello(SYM_NAME(bsp_vsr_table)),gr4
	addi	gr4,(BSP_VEC_STUB_ENTRY*4),gr4
	add	gr4,gr6,gr4
	ldi	@(sp,-12),gr6
	ld	@(gr4,gr0),gr4
	jmpl	@(gr4,gr0)

	/*
	 * Everything except the break trap comes through here.
	 *
	 * On entry:
	 *   sp    -->
	 *   sp-4  --> saved gr4
	 *   sp-8  --> saved gr5
	 *
	 * gr4: destroyed.
	 * gr5: vsr# * 4
	 *
	 */
FUNC_START _exc_trampoline
	/* temp save gr4,gr5,ccr */
	movsg	ccr,gr4
	sti	gr4,@(sp,-12)
	/* temp save gr6 while we construct pic safe address */
	sti	gr6,@(sp,-16)
	movsg	lr,gr4
	call	1f
    1:
	movsg	lr,gr6
	movgs	gr4,lr
	sethi	#gprelhi(1b),gr4
	setlo	#gprello(1b),gr4
	sub	gr6,gr4,gr6
	sethi	#gprelhi(SYM_NAME(bsp_vsr_table)),gr4
	setlo	#gprello(SYM_NAME(bsp_vsr_table)),gr4
	add	gr4,gr6,gr4
	ldi	@(sp,-16),gr6
	ld	@(gr4,gr5),gr4
	jmpl	@(gr4,gr0)
FUNC_END _exc_trampoline

	/*
	 * On entry:
	 *   sp    -->
	 *   sp-4  --> saved gr4
	 *   sp-8  --> saved gr5
	 *   sp-12 --> saved ccr
	 *   gr5   --> vsr# * 4
	 */
FUNC_START _default_handler
	/* Get trap number */
	movsg	tbr,gr5
	srli	gr5,4,gr5
	andi	gr5,255,gr5
	setlos  -1,gr6

	/* flash it on LEDs */
	seti	0x12000000,gr4
    9:	
	sti	gr5,@(gr4,0)
	sti	gr5,@(gr4,8)
	seti	0x30000,gr7
    1:
	subicc	gr7,1,gr7,icc0
	bne	icc0,0,1b
	sti	gr6,@(gr4,0)
	sti	gr6,@(gr4,8)
	seti	0x30000,gr7
    1:
	subicc	gr7,1,gr7,icc0
	bne	icc0,0,1b
	bra	9b
FUNC_END _default_handler


	/*
	 * Secondary vectyor table.
	 * This must be placed at start of RAM.
	 */
	.section ".ram_vectors","ax"
	.globl SYM_NAME(bsp_vsr_table)
SYM_NAME(bsp_vsr_table):
	.word SYM_NAME(_default_handler)	/* reserved exception */
	.word SYM_NAME(_default_handler)	/* data store error */
	.word SYM_NAME(_default_handler)	/* insn access MMU miss */
	.word SYM_NAME(_default_handler)	/* insn access error */
	.word SYM_NAME(_default_handler)	/* insn access exception */
	.word SYM_NAME(_default_handler)	/* insn privilege */
	.word SYM_NAME(_default_handler)	/* illegal insn */
	.word SYM_NAME(_default_handler)	/* FP disabled */
	.word SYM_NAME(_default_handler)	/* MP disabled */
	.word SYM_NAME(_default_handler)	/* register exception */
	.word SYM_NAME(_default_handler)	/* memory alignment */
	.word SYM_NAME(_default_handler)	/* FP exception */
	.word SYM_NAME(_default_handler)	/* MP exception */
	.word SYM_NAME(_default_handler)	/* data access error */
	.word SYM_NAME(_default_handler)	/* data access MMU miss */
	.word SYM_NAME(_default_handler)	/* data access exception */
	.word SYM_NAME(_default_handler)	/* division exception */
	.word SYM_NAME(_default_handler)	/* commit exception   */
	.word SYM_NAME(_default_handler)	/* compound exception */
	.word SYM_NAME(_default_handler)	/* syscall */
	.word SYM_NAME(_default_handler)	/* general trap */
	.word SYM_NAME(_default_handler)	/* HW int1 */
	.word SYM_NAME(_default_handler)	/* HW int2 */
	.word SYM_NAME(_default_handler)	/* HW int3 */
	.word SYM_NAME(_default_handler)	/* HW int4 */
	.word SYM_NAME(_default_handler)	/* HW int5 */
	.word SYM_NAME(_default_handler)	/* HW int6 */
	.word SYM_NAME(_default_handler)	/* HW int7 */
	.word SYM_NAME(_default_handler)	/* HW int8 */
	.word SYM_NAME(_default_handler)	/* HW int9 */
	.word SYM_NAME(_default_handler)	/* HW int10 */
	.word SYM_NAME(_default_handler)	/* HW int11 */
	.word SYM_NAME(_default_handler)	/* HW int12 */
	.word SYM_NAME(_default_handler)	/* HW int13 */
	.word SYM_NAME(_default_handler)	/* HW int14 */
	.word SYM_NAME(_default_handler)	/* HW int15 */
	.word SYM_NAME(_default_handler)	/* break */
	.word 0					/* MT debugging hook */
	.word 0					/* stub entry hook   */
	.word 0					/* bsp shared data   */



