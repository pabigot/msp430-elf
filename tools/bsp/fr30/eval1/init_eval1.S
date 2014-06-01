/*
 * init_eval1.S -- Initialization for Fujitsu MB91V101 Evaluation Board
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
#include <bsp/cpu.h>

#define DRAM_TEST 1

	.file "start.S"
	.text
	.p2align 1
/*
 * Byte offsets from start of bus i/f unit.
 */	
#define ASR1   0
#define AMR1   2
#define ASR2   4
#define AMR2   6
#define ASR3   8
#define AMR3   10
#define ASR4   12
#define AMR4   14
#define ASR5   16
#define AMR5   18
#define AMD0   20
#define AMD1   21
#define AMD32  22
#define AMD4   23
#define AMD5   24
#define DSCR   25
#define RFCR   26
#define EPCR0  28
#define EPCR1  30
#define DMCR4  32
#define DMCR5  34

#define DMCR_1K    0x2000
#define DMCR_Q1W   0x0800
#define DMCR_Q4W   0x0400
#define DMCR_DSAS  0x0200
#define DMCR_HYPR  0x0100
#define DMCR_PAGE  0x0080
#define DMCR_CW    0x0040
#define DMCR_SLFR  0x0020
#define DMCR_REFE  0x0010
	

	.macro  led val
	ldi:32	#0x00304000,r12
	ldi:8	#\val,r11
	stb	r11,@r12
	.endm

	.macro delay
	ldi:32	#0x4000,r12
    9:
	add2	#-1,r12
	bp	9b
	.endm

FUNC_START _bsp_init_stack
	/*
	 * First thing to do is to set up external chip selects.
	 */
	ldi:20	#0x60c,r14		/* start address of BUS I/F unit */

	/* CS0 => ROM/FLASH */
	ldi:8	#0x0b,r0
	stb	r0,@(r14,AMD0)	/* CS0 is 16bit bus with 3 cycle wait */

	/* CS1 => External I/O (64K) 0x00300000 - 0x003fffff */
	ldi:8	#0x30,r0
	sth	r0,@(r14,ASR1)
	ldi:8	#0x0f,r0
	sth	r0,@(r14,AMR1)
	ldi:8	#0x09,r0
	stb	r0,@(r14,AMD1)	/* CS1 is 16bit bus with 2 cycle wait */

	/* CS2 => SRAM (512K) 0x00600000 - 0x0067ffff */
	ldi:8	#0x60,r0
	sth	r0,@(r14,ASR2)
	ldi:8	#0x07,r0
	sth	r0,@(r14,AMR2)
	ldi:8	#0x40,r0
	stb	r0,@(r14,AMD32)	/* CS2 is 16bit bus with 0 cycle wait */

	/* CS4 => EDO DRAM (4MB) 0x01000000 - 0x013fffff */
	ldi:8	#0x4d,r0
	stb	r0,@(r14,DSCR)	/* enable RAS0/CS0L/CS0H/DW0 */
	ldi:20	#0x1804,r0	/* refresh cycle every 24 clocks = 15.36us */
	sth	r0,@(r14,RFCR)  /* assume 50Mhz, modify below if not */
	ldi:20	# DMCR_1K+DMCR_HYPR+DMCR_PAGE+DMCR_CW+DMCR_REFE,r0
	sth	r0,@(r14,DMCR4)
	ldi:20	#0x0100,r0
	sth	r0, @(r14,ASR4)
	ldi:8	#0x3f,r0
	sth	r0,@(r14,AMR4)
	ldi:8	#0x88,r0
	stb	r0,@(r14,AMD4)	/* CS4 is 16bit bus with 0 cycle wait */

	/* setup output pins */
	ldi:20	#0xedf,r0	/* enable RDY, disable CS5 */
	sth	r0,@(r14,EPCR0)

	/* write to mode register */
	ldi:20	#0x7ff,r14
	ldi:8	#0x80,r0
	stb	r0,@r14

	led	0x0
	delay

	/*
	 * read SW1 to get desired CLK setting
	 *
	 * SW1-7 Sw1-6 SW1-5    PLL    CPU    BUS
	 * ----- ----- -----   -----  -----  -----
	 *  OFF   OFF   OFF    50.00  50.00  25.00
	 *  OFF   OFF    ON    25.00  25.00  12.50
	 *  OFF    ON   OFF    12.50  12.50   6.25
	 *  OFF    ON    ON    25.00  25.00  25.00
	 *   ON   OFF   OFF    25.00  12.50  12.50
	 *   ON   OFF    ON    25.00   6.25   6.25
	 *   ON    ON   OFF    12.50  12.50  12.50
	 *   ON    ON    ON    12.50   6.25   6.25
	 */
	ldi:32	#0x00304000,r8
	ldub	@r8,r8
	lsr	#4,r8
	ldi:8   7,r9
	and	r9,r8
	bra	set_clk

    _gcr_tbl:
	.byte 0x47, 0x03, 0x8b, 0x47, 0x03, 0x07, 0x07, 0x07
	
    _pll_tbl:
	.byte 0x00, 0x00, 0x40, 0x40, 0x40, 0x00, 0x40, 0x80

    _dbl_tbl:
	.byte 0, 0, 0, 0, 0, 1, 1, 1

	/*
	 * Setup clocks.
	 * At this point r8 has offset into _gcr_tbl, _pll_tbl, and _dbl_tbl.
	 */
    set_clk:
	ldi:20	#0x484,r0	/* GCR register */
	ldi:20	#0x488,r1	/* PCTR register */

	/* set GCR */
	ldi:32	#_gcr_tbl,r9
	add	r8,r9
	ldub	@r9,r2
	stb	r2,@r0
	nop
	nop
	nop
	nop
	
	/*
	 * VCO wait
	 */
	ldub	@r1,r5		/* get PCTR */
	ldi:8	#8,r3
	and	r5,r3
	beq	2f
	ldi:20	#0x15e,r2	/* wait loop count */
    1:
	add2	#-1,r2
	bne	1b
    2:

	
	ldi:32	#_dbl_tbl,r9
	add	r8,r9
	ldub	@r9,r2
	cmp	#0,r2
	beq	set_pll
	
	/*
	 * doubler on
	 */
	borh	#1,@r0
    1:
	btsth	#2,@r0
	beq	1b
	/*
	 * VCO wait
	 */
	ldub	@r1,r5		/* get PCTR */
	ldi:8	#8,r3
	and	r5,r3
	beq	set_pll
	ldi:20	#0x15e,r2	/* wait loop count */
    1:
	add2	#-1,r2
	bne	1b

    set_pll:
	/* set pll freq */
	ldi:32	#_pll_tbl,r9
	add	r8,r9
	ldub	@r9,r2
	stb	r2,@r1

	bandl	#0x0e,@r0	/* switch clock to PLL */
	
	nop
	nop
	nop
	nop
	nop

	/* turn off all LEDs */
	led	0xff
	delay

#if DRAM_TEST	
	ldi:32	#0x01000000,r4	/* DRAM base */
	ldi:8	#0,r0		/* data */
	ldi:32	#80000,r1	/* number of bytes / per loop  */
	ldi:8	#8,r2		/* number of loops */
	ldi:8	#1,r5
	ldi:8	#0xff,r6

    1:
	st	r0,@r4
	add2	#-4,r1
	bne	1b
	
	ldi:32	#0x00304000,r7
	eor	r5,r6
	stb	r6,@r7
	lsl	#1,r5
	
	ldi:32	#80000,r1	/* number of bytes / per loop  */
	add2	#-1,r2
	bne	1b
	

	ldi:32	#0x01000000,r4	/* DRAM base */
	ldi:32	#0x01000004,r14	/* DRAM base */
	ldi:8	#0,r1
	ldi:8	#8,r2		/* number of loops */
	ldi:8	#1,r5
	ldi:8	#0,r6
    1:
	ldi:32	#0x55aa33cc,r0
	st	r0,@r4		/* store test pat */
	delay
	st	r1,@r14		/* put zero out on the bus */
	ld	@r4,r8
	cmp	r8,r0
	bne	2f
	
	ldi:32	#0x00304000,r7
	or	r5,r6
	stb	r6,@r7
	
    2:
	lsl	#1,r5

	ldi:20	#0x80000,r0
	add	r0,r4
	add2	#-1,r2
	bne	1b
	
	delay
#endif

	/* setup initial stack pointer */
	ldi:8	#0,r0
	mov	r0,USP		/* clear USP */
	ldi:32	#0x013ffff0,r0
	mov	r0,SSP		/* initial SSP */
	
	ret
FUNC_END _bsp_init_stack
