/*
 * init_86x.S -- Initialization for Fujitsu MB86831 Motherboard.
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
	.file "init_86x.S"
	
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "mb8683x.h"

#define DRAM_BASE      0x04000000	/* base of system DRAM */
#define CS3_BASE       0x00000000       /* base of internal resource regs */
#define CS3_ASI        7                /* ASI of internal resource regs  */
	
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

	/*
	 * Put this in a special section so that the linker script
	 * can place it early in the final image. That is necessary
	 * because the boot ROM can only run in the first 32K until
	 * the address mask gets set.
	 */
	.section ".bsp_init","ax"
	.align     2

FUNC_START _bsp_init_stack
	
	/*
	 * First, setup chip selects.
	 *
	 * NB: The AMR_VAL macro actually inverts the mask bits. For me, it is
	 *     more natural to write a 1 bit where I want the address compared.
	 *     The sparc registers use 0 bits, instead.
	 */
	
	/* -CS0 ADDR_MASK:0xfc000000 ASI_MASK:0xfc */
	set	AMR_VAL(0xfc,0xfc000000),%l0
	mov	AMR0,%l1
	sta	%l0,[%l1] 1
	
	/* -CS1	BASE:0x10000000 ASI:4 */
	set	ARSR_VAL(4,0x10000000),%l0
	mov	ARSR1,%l1
	sta	%l0,[%l1] 1
	/* -CS1 ADDR MASK:0xf0000000 ASI MASK:0x7 */
	set	AMR_VAL(7,0xf0000000),%l0
	mov	AMR1,%l1
	sta	%l0,[%l1] 1
	
	/* -CS2	BASE:0x20000000 ASI:4 */
	set	ARSR_VAL(4,0x20000000), %l0
	mov	ARSR2,%l1
	sta	%l0,[%l1] 1
	/* -CS2 ADDR MASK:0xf0000000 ASI MASK:0x7 */
	set	AMR_VAL(7,0xf0000000),%l0
	mov	AMR2,%l1
	sta	%l0,[%l1] 1
	
	/* -CS3	BASE:CS3_BASE ASI:CS3_ASI */
	set	ARSR_VAL(CS3_ASI,CS3_BASE),%l0
	mov	ARSR3,%l1
	sta	%l0,[%l1] 1
	/* -CS3 ADDR MASK:0xffff0000 ASI MASK:0x7 */
	set	AMR_VAL(7,0xffff0000),%l0
	mov	AMR3,%l1
	sta	%l0,[%l1] 1
	
	/* -CS4 BASE: DRAM_BASE ASI:0xb */
	set	ARSR_VAL(0xb,DRAM_BASE),%l0
	mov	ARSR4,%l1
	sta	%l0,[%l1] 1
	/* -CS4 ADDR MASK:0xfc000000 ASI MASK:0xfc */
	set	AMR_VAL(0xfc,0xfc000000),%l0
	mov	AMR4,%l1
	sta	%l0,[%l1] 1
	
	/* -CS5 BASE:0x30000000 ASI:0xb */
	set	ARSR_VAL(0xb,0x30000000),%l0
	mov	ARSR5,%l1
	sta	%l0,[%l1] 1
	/* -CS5 ADDR MASK:0xfff80000 ASI MASK:0xfc */
	set	AMR_VAL(0xfc,0xfff80000),%l0
	mov	AMR5,%l1
	sta	%l0,[%l1] 1

	
	/*
	 * Setup wait states. Each wait state register sets the wait states for
	 * a pair of chip selects. The lower bits hold the wait state info for
	 * the lower numbered chip select.
	 */

	/* -CS0: 5 wait states,  -CS1: 7 wait states */
	set	WSSR_VAL(7,7,WSSR_WAITEN,5,5,WSSR_WAITEN),%l0
	mov	WSSR0,%l1
	sta	%l0,[%l1] 1

	/* -CS2: wait states disabled,  -CS3: wait states disabled */
	set	WSSR_VAL(0,0,0,0,0,0),%l0
	mov	WSSR1,%l1
	sta	%l0,[%l1] 1

	/* -CS4: wait states disabled,  -CS5: 0 wait states */
	set	WSSR_VAL(0,0,WSSR_WAITEN|WSSR_OVERRIDE,0,0,0),%l0
	mov	WSSR2,%l1
	sta	%l0,[%l1] 1

	led	0x7f

	/* clear cache/BIU control register */
	mov	CBIR,%l1
	sta	%g0,[%l1] 1

	/* Read clock switch to determine the value of the refresh timer */
	sethi	%hi(CLKSW_ADDR),%l1
	ldub	[%l1 + %lo(CLKSW_ADDR)],%l0
	btst	0x80,%l0
	bne,a	1f
	 mov	10,%l0		/* force to 10MHz if CLKSW-8 is ON */
    1:
	umul	%l0,15,%l0
	mov	DRLD,%l1
	sta	%l0,[%l1] 1
	mov	REFTMR,%l1
	sta	%l0,[%l1] 1

	/* read SW1 to get DRAM page size */
	sethi	%hi(SW1_ADDR),%l1
	ldub	[%l1 + %lo(SW1_ADDR)],%l0
	btst	0x10,%l0
	be,a	1f
	 mov	0x0e,%l0	/* 1K page if branch taken (SW1-5 is OFF) */
	mov	0x06,%l0	/* 2K page (SW1-5 is OFF) */
    1:
	mov	SPGMR,%l1
	sta	%l0,[%l1] 1

	led	0xbf

	/* Turn on all system services */	
	mov	SSCR_TIMER|SSCR_WAIT|SSCR_CS|SSCR_SAMEPG,%l0
	mov	SSCR,%l1
	sta	%l0,[%l1] 1
	nop
	nop
	nop
	nop
	
	led	0xdf
	
	/*
	 * Initialize caches.
	 */
	sethi	%hi(0x1000),%l0		/* bank 1 invalidate */
	sethi	%hi(0x80000000),%l1	/* bank 2 invalidate */
	mov	3,%l2			/* clear lock, lru, and valid bits */ 
	sta	%l2,[%l0] 0xc		/* do it - icache bank 1 */
	sta	%l2,[%l0] 0xe		/* do it - dcache bank 1 */
	sta	%l2,[%l0 + %l1] 0xc	/* do it - icache bank 2 */
	sta	%l2,[%l0 + %l1] 0xe	/* do it - dcache bank 2 */
	
	/* now, enable caches and buffers */
	mov	CBIR_ICEN|CBIR_DCEN|CBIR_PBEN|CBIR_WBEN,%l0
	mov	CBIR,%l1
	sta	%l0,[%l1] 1
	nop 
	nop 
	nop 
	nop
	
	/* enable data and insn bursts */
	mov	BCR_IBE|BCR_DBE,%l0
	mov	BCR,%l1
	sta	%l0,[%l1] 1
	nop 
	nop 
	nop 
	nop

	/*
	 * DRAM setup/test.
	 */
	led 0xef

	/*
	 * Test SW1-7 to determine normal or EDO mode.
	 *   SW1-7 ON  = EDO
	 *   SW1-7 OFF = Normal.
	 */
	sethi	%hi(SW1_ADDR),%l1
	ldub	[%l1 + %lo(SW1_ADDR)],%l7
	mov	DBANKR_SA04,%l0		/* DRAM starts at 0x04000000 */
	btst	0x40,%l7
	bne	1f			/* branch if SW1-7 is OFF */
	 mov	SSCR_DRAM,%l1
	/* EDO DRAM, enable burst in SSCR and EDO in DBANKR */
	or	%l1,SSCR_BURST,%l1
	or	%l0,DBANKR_EDO,%l0
    1:
	/*
	 * Now, test SW1 to get DRAM page and bank size.
	 *   SW1-5 ON  = 2k page, 16MB bank. (up to 64MB total)
	 *   SW1-5 OFF = 1k page, 4MB bank.  (up to 16MB total)
	 */
	btst	0x10,%l7
	bne,a	1f				/* branch if OFF */
	 or	%l0,DBANKR_4M|DBANKR_CA10,%l0  /* 1K page */
	or	%l0,DBANKR_16M|DBANKR_CA11,%l0 /* 2K page */
    1:
	mov	CS3_BASE+DBANKR,%l2
	sta	%l0,[%l2] CS3_ASI
	
	mov	DTIMR_RPS2|DTIMR_CBR3|DTIMR_CAS2|DTIMR_RP2,%l0
	mov	CS3_BASE+DTIMR,%l2
	sta	%l0,[%l2] CS3_ASI
	
	mov	SSCR,%l2
	lda	[%l2] 1, %l0
	or	%l0,%l1,%l0
	sta	%l0,[%l2] 1

	/*
	 * Test SW1 to get potential DRAM limit.
	 *   SW1-5 ON  = 2k page, up to 64MB total
	 *   SW1-5 OFF = 1k page, up to 16MB total
	 */
	btst	0x10,%l7
	bne,a	1f				/* branch if OFF */
	 sethi	%hi(DRAM_BASE + 16*1024*1024),%l0
	sethi	%hi(DRAM_BASE + 64*1024*1024),%l0
    1:

	/* subtract 4 to get last valid DRAM address */
	add	%l0,-4,%l0
		
	/* Assume maximim memory and fill with pattern */	
	sethi	%hi(DRAM_BASE),%l2
	set	0xaaaaaaaa,%l3
    1:
	st	%l3,[%l2]
	cmp	%l2,%l0
	blt	1b
	 add	%l2,4,%l2

	/*
	 * Go back, read data and compare with written data.
	 * Fill in with zero as we go along.
	 */
	sethi	%hi(DRAM_BASE),%l2
    1:
	ld	[%l2],%l4
	cmp	%l4,%l3
	bne	2f
	 st	%g0,[%l2]
	cmp	%l2,%l0
	blt,a	1b
	 add	%l2,4,%l2
    2:
	led	0xff
	
	sub	%l2,64,%sp
	sethi	%hi(DRAM_BASE),%l1
	sub	%l2,%l1,%l0
	st	%l0,[%sp]

	retl
	 nop

FUNC_END _bsp_init_stack

