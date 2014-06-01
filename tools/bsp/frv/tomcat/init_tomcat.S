/*
 * init_tomcat.S -- Initialization for tomcat EVB.
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
	.file "init_tomcat.S"
	
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
	
	.text

FUNC_START _bsp_init_stack
	/* setup hsr0 */
	movsg	hsr0,gr4
	seti	~(HSR_ICE|HSR_DCE),gr5
	and	gr4,gr5,gr6
	seti	HSR_FRHE|HSR_FRLE|HSR_GRHE|HSR_GRLE,gr5
	or	gr6,gr5,gr4
	movgs	gr4,hsr0

	/* Upper half of internal and companion register addresses */
	sethi	#0xfeff,gr8
	
	/* Set SDRAM start address to 0x04000000 */ 
	setlo	%lo(SDRAM_DARS0),gr8
/*	setlos	#0x400,gr4*/
	setlos	#0x000,gr4
	st	gr4,@(gr8,gr0)

	/* Set SDRAM size to 32MB */ 
	setlo	%lo(SDRAM_DAMK0),gr8
	setlos	#0x1f,gr4
	st	gr4,@(gr8,gr0)

	setlo	%lo(COMP_RCBCFG),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(SYSCTL_TCBCFG),gr8
	setlos	#1,gr4
	st	gr4,@(gr8,gr0)
	
	setlo	%lo(SYSCTL_RCBCFG),gr8
	setlos	#1,gr4
	st	gr4,@(gr8,gr0)
	
	setlo	%lo(COMP_TCBCFG),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(COMP_RWDBCFG),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(SYSCTL_TWDBCFG),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(COMP_RRDBCFG),gr8
	st	gr0,@(gr8,gr0)

	setlo	%lo(SYSCTL_TRDBCFG),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(SYSCTL_RDBCFG),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(COMP_TDBCFG),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(COMP_AEFMR),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(COMP_LMXEMR),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(COMP_LWTCR),gr8
	st	gr0,@(gr8,gr0)
	
	setlo	%lo(SDRAM_DAMC),gr8
	setlos	#0x100,gr4
	st	gr4,@(gr8,gr0)

	setlo	%lo(SDRAM_DCTL),gr8
	seti	(DCTL_TRAS_5|DCTL_TRP_3|DCTL_TRCD_3|DCTL_TRRD_2),gr4
	st	gr4,@(gr8,gr0)
	
	setlo	%lo(SDRAM_DCFG),gr8
	st	gr0,@(gr8,gr0)
	
	/* 4 banks, 12 row address bits, 8 column address bits */
	setlo	%lo(SDRAM_DAN),gr8
	seti	0x28282828,gr4
	st	gr4,@(gr8,gr0)

	/* refresh timer */	
	setlo	%lo(SDRAM_DART),gr8
	seti	310,gr4
	st	gr4,@(gr8,gr0)

	/* enable refresh */	
	setlo	%lo(SDRAM_DRCN),gr8
	seti	(DRCN_RE|DRCN_ARS_4K|DRCN_SRN_1X),gr4
	st	gr4,@(gr8,gr0)
	
	/* set mode */
	setlo	%lo(SDRAM_DMS),gr8
	seti	(DMS_BL_8|DMS_CL_3),gr4
	st	gr4,@(gr8,gr0)

	DELAY
	LED 0xf0

#if 0	
	/* test sram */
	sethi	0x3000,gr9
	setlo	0x0000,gr9
	sethi	0x3020,gr10
	setlo	0x0000,gr10

	sethi	0xaaaa,gr4
	setlo	0xaaaa,gr4

	/* fill in with value */
	or	gr9,gr0,gr8
    1:
	st	gr4,@(gr8,gr0)
	addi	gr8,1024,gr8
	subcc	gr8,gr10,gr0,icc0
	bne	icc0,0,1b

	sti	gr4,@(gr9,0)
	sti	gr0,@(gr9,4)
	DELAY
	ldi	@(gr9,0),gr5
	subcc	gr4,gr5,gr0,icc0
	beq	icc0,0,5f
	
    9:
	LED 0xff
	DELAY
	LED 0x00
	DELAY
	bra	9b
    5:
	LED 0xaa
	DELAY
	LED 0x00
	DELAY
	bra	5b
#else
	seti	(32 << 20),gr4
	subi	gr4,64,sp
	st	gr4,@(sp,gr0)
#endif
	ret
FUNC_END _bsp_init_stack


#ifdef HAVE_BSP_CLOCK
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
	/* mb86941_write(DB_BASE, MB86941_IRQCLR, (1 << irq)); */
	sethi	0x1100,gr4
	setlo	(0x03 << 2),gr4
	seti	(1<<8),gr5
	sth	gr5,@(gr4,gr0)
	nop

	/* clear latch */
	/* mb86941_write(DB_BASE, MB86941_IRLATCH, 0x10); */
	setlo	(0x05 << 2),gr4
	seti	0x10,gr5
	sth	gr5,@(gr4,gr0)

	/* restore regs and return */
	ldi	@(sp,-16),gr17
	ldi	@(sp,-12),gr4
	movgs	gr4,ccr
	ldi	@(sp,-4),gr4
	ldi	@(sp,-8),gr5
	rett	0
FUNC_END   _bsp_clock_vsr
#endif
