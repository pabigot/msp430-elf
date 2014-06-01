/*
 * init_tiger.S -- Initialization for tiger EVB.
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
	.file "init_tiger.S"
	
#include <bsp/cpu.h>
#include "fr500.h"

#define SWITCH3 0x1200000c

	.macro DELAY
	sethi	#2,gr8
	setlo	#0,gr8
     1:
	subicc	gr8,1,gr8,icc0
	bne	icc0,0x0,1b
	.endm

	.text
FUNC_START _bsp_init_stack
	movsg	lr,gr19

	/* setup hsr0 */
	movsg	hsr0,gr4
	seti	~(HSR_ICE|HSR_DCE),gr5
	and	gr4,gr5,gr6
	seti	HSR_FRHE|HSR_FRLE|HSR_GRHE|HSR_GRLE,gr5
	or	gr6,gr5,gr4
	movgs	gr4,hsr0

	/* enable memory_address_not_aligned exception */
	movsg	isr,gr4
	setlos	0xfffffffe,gr5
	and	gr4,gr5,gr4
	movgs	gr4,isr

	/* Upper half of internal and companion register addresses */
	sethi	#0xfeff,gr8

	/* setup some companion chip registers */
	setlo	%lo(0x8808),gr8
	sethi	0x2000,gr4
	setlo	0x1000,gr4
	st	gr4,@(gr8,gr0)

	setlo	%lo(0x8838),gr8
	setlos	0x2ff,gr4
	st	gr4,@(gr8,gr0)

	setlo	%lo(0x8868),gr8
	setlos	0x7ff4,gr4
	st	gr4,@(gr8,gr0)
		
	/* Set SDRAM start address to 0x00000000 */ 
	setlo	%lo(SDRAM_DARS0),gr8
	st	gr0,@(gr8,gr0)

	/* Set SDRAM size to 32MB */ 
	setlo	%lo(SDRAM_DAMK0),gr8
	setlos	#0x1f,gr4
	st	gr4,@(gr8,gr0)

	sethi	%hi(SWITCH3),gr4
	setlo	%lo(SWITCH3),gr4
	ld	@(gr4,gr0),gr4
	xori	gr4,0xff,gr4
	andi	gr4,0x1f,gr4
	setlos	1,gr5
	sll	gr5,gr4,gr11

#define MASK1 0xe7eee00
	
	sethi	%hi(MASK1),gr4
	setlo	%lo(MASK1),gr4
	andcc	gr11,gr4,gr0,icc0
	beq	icc0,0x0,1f
	
	/* Set SDRAM bank1 start address to 0x04000000 */ 
	setlo	%lo(SDRAM_DARS1),gr8
	setlos	0x400,gr4
	st	gr4,@(gr8,gr0)
	setlo	%lo(SDRAM_DAMK1),gr8
	setlos	#0x1f,gr4
	st	gr4,@(gr8,gr0)
    1:

#define MASK2 0xe7ee000

	sethi	%hi(MASK2),gr4
	setlo	%lo(MASK2),gr4
	andcc	gr11,gr4,gr0,icc1
	setlos	0,gr12
	beq	icc1,0x0,1f
	setlos	4,gr12
    1:
	sethi	%hi(COMP_TCBCFG),gr13
	setlo	%lo(COMP_TCBCFG),gr13
	sethi	%hi(SYSCTL_TCBCFG),gr14
	setlo	%lo(SYSCTL_TCBCFG),gr14
	call	SYM_NAME(__cbcfg)
	sethi	%hi(SYSCTL_RCBCFG),gr13
	setlo	%lo(SYSCTL_RCBCFG),gr13
	sethi	%hi(COMP_RCBCFG),gr14
	setlo	%lo(COMP_RCBCFG),gr14
	call	SYM_NAME(__cbcfg)
	
	setlos	0,gr12
	beq	icc1,0x0,1f
	sethi	0x4000,gr12
    1:
	sethi	%hi(COMP_RWDBCFG),gr13
	setlo	%lo(COMP_RWDBCFG),gr13
	sethi	%hi(SYSCTL_TWDBCFG),gr14
	setlo	%lo(SYSCTL_TWDBCFG),gr14
	call	SYM_NAME(__dbcfg)
	sethi	%hi(COMP_RRDBCFG),gr13
	setlo	%lo(COMP_RRDBCFG),gr13
	sethi	%hi(SYSCTL_TRDBCFG),gr14
	setlo	%lo(SYSCTL_TRDBCFG),gr14
	call	SYM_NAME(__dbcfg)
	sethi	%hi(COMP_TDBCFG),gr13
	setlo	%lo(COMP_TDBCFG),gr13
	sethi	%hi(SYSCTL_RDBCFG),gr14
	setlo	%lo(SYSCTL_RDBCFG),gr14
	call	SYM_NAME(__dbcfg)
	
	setlo	%lo(COMP_AEFMR),gr8
	st	gr0,@(gr8,gr0)
	
	bne	icc1,0x0,1f
	setlo	%lo(COMP_LMXEMR),gr8
	st	gr0,@(gr8,gr0)
	setlo	%lo(COMP_LWTCR),gr8
	st	gr0,@(gr8,gr0)
	bra	2f
    1:
	setlos	-1,gr4
	setlo	%lo(COMP_PCNTL),gr8
	st	gr4,@(gr8,gr0)
	setlo	0x8810,gr8
	sethi	0x2000,gr4
	setlo	0x5000,gr4
	st	gr4,@(gr8,gr0)
	setlo	0x8840,gr8
	setlos	0x0fff,gr4
	st	gr4,@(gr8,gr0)
	setlo	0x8870,gr8
	setlos	0x7ff4,gr4
	st	gr4,@(gr8,gr0)
	setlo	0x8828,gr8
	sethi	0x2000,gr4
	setlo	0x3000,gr4
	st	gr4,@(gr8,gr0)
	setlo	0x8858,gr8
	setlos	0xff,gr4
	st	gr4,@(gr8,gr0)
	setlo	0x8888,gr8
	setlos	0x7ff4,gr4
	st	gr4,@(gr8,gr0)
	
	setlo	0x88a0,gr8
	st	gr0,@(gr8,gr0)
    2:
	sethi	%hi(MASK1),gr5
	setlo	%lo(MASK1),gr5
	andcc	gr11,gr5,gr0,icc0
	setlos	0x100,gr4
	beq	icc0,0x0,1f
	setlos	0x0,gr4
    1:
	setlo	%lo(SDRAM_DAMC),gr8
	st	gr4,@(gr8,gr0)

#define MASK3 0xe7e0000
	sethi	%hi(MASK3),gr5
	setlo	%lo(MASK3),gr5
	andcc	gr11,gr5,gr0,icc1
	sethi	0xa,gr4
	setlo	0x4220,gr4
	beq	icc1,0x0,1f
	sethi	0xc,gr4
	setlo	0x6324,gr4
    1:
	setlo	%lo(SDRAM_DCTL),gr8
	st	gr4,@(gr8,gr0)

	setlo	%lo(SDRAM_DCFG),gr8
	st	gr0,@(gr8,gr0)

	sethi	0x2828,gr4
	setlo	0x2828,gr4
	setlo	%lo(SDRAM_DAN),gr8
	st	gr4,@(gr8,gr0)

	movsg	lr,gr5
	call	1f
	
	/* SDRAM refresh table */
	.word	0x136
	.word	0x136
	.word	0x206
	.word	0x26f
	.word	0x26f
	.word	0x26f
	.word	0x26f
	.word	0x26f
	.word	0x26f
	.word	0x184
	.word	0x183
	.word	0x182
	.word	0x184
	.word	0x184
	.word	0x183
	.word	0x182
	.word	0x40f
	.word	0x40f
	.word	0x40e
	.word	0x40d
	.word	0x618
	.word	0x618
	.word	0x618
	.word	0x618
	.word	0x821
	.word	0x821
	.word	0x821
	.word	0x821
	.word	0x821
	.word	0x821
	.word	0x821
	.word	0x821
    1:
	movsg	lr,gr4
	movgs	gr5,lr

	sethi	%hi(SWITCH3),gr5
	setlo	%lo(SWITCH3),gr5
	ld	@(gr5,gr0),gr5
	xori	gr5,0xff,gr5
	andi	gr5,0x1f,gr5
	slli	gr5,2,gr5
	ld	@(gr4,gr5),gr4
	
	setlo	%lo(SDRAM_DART),gr8
	st	gr4,@(gr8,gr0)

	setlos  0x100,gr4
	setlo	%lo(SDRAM_DRCN),gr8
	st	gr4,@(gr8,gr0)

	setlos	0x2300,gr4
	beq	icc1,0x0,1f
	setlos	0x3300,gr4
    1:
	setlo	%lo(SDRAM_DMS),gr8
	st	gr4,@(gr8,gr0)

#define MASK4 0xe7eee0f

	sethi	%hi(MASK4),gr4
	setlo	%lo(MASK4),gr4
	sethi	%hi(SWITCH3),gr5
	setlo	%lo(SWITCH3),gr5
	ld	@(gr5,gr0),gr5
	xori	gr5,0xff,gr5
	andi	gr5,0x1f,gr5
	srl	gr4,gr5,gr4
	andicc	gr4,1,gr0,icc0
	bne	icc0,0x0,1f
	setlos	0xff,gr8
	call	SYM_NAME(set_led)
    9:
	bra	9b
    1:
	setlos	0xfe,gr8
	call	SYM_NAME(set_led)

	DELAY

	/* set initial stack value */
	seti	0x30200000,gr4
	subi	gr4,64,sp
	st	gr4,@(sp,gr0)

	/* enable icache */
	movsg	hsr0,gr5
	seti	HSR_ICE,gr4
	or	gr5,gr4,gr5
	bar
	movgs	gr5,hsr0
	bar

#if 0
	setlos	0xfd,gr8
	call	SYM_NAME(set_led)

	/* enable dcache */
	movsg	hsr0,gr5
	seti	HSR_DCE,gr4
	or	gr5,gr4,gr5
	bar
	movgs	gr5,hsr0
	bar

#if 0
	setlos	0x80,gr8
	call	SYM_NAME(set_led)

	seti	0x00000000, gr4
	ld	@(gr4,gr0),gr0
	
	setlos	0x81,gr8
	call	SYM_NAME(set_led)
	
	st	gr0, @(gr4,gr0)

	setlos	0x82,gr8
	call	SYM_NAME(set_led)
	
	ldi	@(gr4,66),gr0
	
	setlos	0x83,gr8
	call	SYM_NAME(set_led)
	
	sti	gr0,@(gr4,166)
#endif
#endif
	setlos	0x00,gr8
	call	SYM_NAME(set_led)

	movgs	gr19,lr
	ret
FUNC_END _bsp_init_stack


	/*
	 * Helper subroutine to write pairs of command buffer config registers.
	 *
	 * Input: gr12 = value to write
	 *        gr13 = reg1 address
	 *        gr14 = reg2 address
	 *
	 * Destroyed: gr4
	 */
FUNC_START __cbcfg
	ld	@(gr13,gr0),gr4
	andi	gr4,15,gr4
	subcc	gr12,gr4,gr0,icc0
	bhi	icc0,0x0,2f
    1:
	st	gr12,@(gr14,gr0)
	ld	@(gr14,gr0),gr4
	andi	gr4,15,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,1b
    1:
	st	gr12,@(gr13,gr0)
	ld	@(gr13,gr0),gr4
	andi	gr4,15,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,1b
	ret
    2:
	st	gr12,@(gr13,gr0)
	ld	@(gr13,gr0),gr4
	andi	gr4,15,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,2b
    1:
	st	gr12,@(gr14,gr0)
	ld	@(gr14,gr0),gr4
	andi	gr4,15,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,1b
	ret
FUNC_END __cbcfg


	/*
	 * Helper subroutine to write pairs of data buffer config registers.
	 *
	 * Input: gr12 = value to write
	 *        gr13 = reg1 address
	 *        gr14 = reg2 address
	 *
	 * Destroyed: gr4,gr5,gr6
	 */
FUNC_START __dbcfg
	sethi	0xc000,gr6
	setlo	0x0003,gr6
	
	ld	@(gr13,gr0),gr4
	andi	gr4,3,gr4
	andi	gr12,3,gr5
	subcc	gr5,gr4,gr0,icc0
	bhi	icc0,0x0,2f
	bc	icc0,0x0,1f
	srlicc	gr12,30,gr0,icc0
	bne	icc0,0x0,2f
    1:
	st	gr12,@(gr14,gr0)
	ld	@(gr14,gr0),gr4
	and	gr4,gr6,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,1b
    1:
	st	gr12,@(gr13,gr0)
	ld	@(gr13,gr0),gr4
	and	gr4,gr6,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,1b
	ret
    2:
	st	gr12,@(gr13,gr0)
	ld	@(gr13,gr0),gr4
	and	gr4,gr6,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,2b
    1:
	st	gr12,@(gr14,gr0)
	ld	@(gr14,gr0),gr4
	and	gr4,gr6,gr4
	subcc	gr12,gr4,gr0,icc0
	bne	icc0,0x0,1b
	ret
FUNC_END __dbcfg


FUNC_START set_led
	sethi	%hi(SWITCH3),gr4
	setlo	%lo(SWITCH3),gr4
	ld	@(gr4,gr0),gr4
	xori	gr4,255,gr4
	slli	gr4,8,gr4
	andi	gr8,255,gr8
	or	gr8,gr4,gr8
	sethi	0x1200,gr4
	setlo	0x0,gr4
	sti	gr8,@(gr4,0)
	sti	gr8,@(gr4,8)
	ret
FUNC_END set_led

	/*
	 * Low-level handler for clock interrupts.
	 * Just increment _bsp_clock and clear IRC state.
	 *
	 * On entry:
	 *   sp    -->
	 *   sp-4  --> gr4
	 *   sp-8  --> gr5
	 *   sp-12 --> ccr
	 *   gr4   --> unknown
	 *   gr5   --> vsr# * 4
	 */
FUNC_START _bsp_clock_vsr
	sti	gr17,@(sp,-16)

	movsg	lr,gr4
	call	1f
    1:
	movsg	lr,gr17
	movgs	gr4,lr
	sethi	%gprelhi(1b),gr5
	setlo	%gprello(1b),gr5
	sub	gr17,gr5,gr17
	sethi	%gprelhi(SYM_NAME(_bsp_clock)),gr4
	setlo	%gprello(SYM_NAME(_bsp_clock)),gr4
	ld	@(gr17,gr4),gr5
	addi	gr5,1,gr5
	st	gr5,@(gr17,gr4)

	/* Read CNT2 register to clear interrupt source */
	sethi	0x1000,gr4
	setlo	(0x1b << 2),gr4
	lduh	@(gr4,gr0),gr5
	
	/* clear interrupt */
	/* comp_write(COMP_RC,0x10000<<irq) */
	sethi	%hi(COMP_RC),gr4
	setlo	%lo(COMP_RC),gr4
	seti	(0x10000<<8),gr5
	st	gr5,@(gr4,gr0)
	nop

	/* clear latch */
	/* comp_write(COMP_IRL, 0x100000); */
	setlo	%lo(COMP_IRL),gr4
	seti	0x100000,gr5
	st	gr5,@(gr4,gr0)

	/* restore regs and return */
	ldi	@(sp,-16),gr17
	ldi	@(sp,-12),gr4
	movgs	gr4,ccr
	ldi	@(sp,-4),gr4
	ldi	@(sp,-8),gr5
	rett	0
FUNC_END   _bsp_clock_vsr

