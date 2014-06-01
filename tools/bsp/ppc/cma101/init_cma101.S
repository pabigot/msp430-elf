/*
 * init_cma101.S -- Initialization for Cogent Computer Systems CMA286 MPC860/821 Module
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
	.file "init_cma101.S"
	.text

#include <bsp/cpu.h>
	
        .align     2
	
FUNC_START _bsp_init_stack
	
	/*
	 * Base of internal map is 0xFF000000.
	 */
	lis	r4,0xFF00
	mtspr	immr,r4
		
	/*
	 * Disable Watchdog timer.
	 */
	lwi	r3,0x00000000
	stw	r3,SYPCR(r4)
	
	/*
	 * Clear REFA & REFB. Enable but freeze timebase.
	 */
	lwi	r3,0x00c3
	sth	r3,TBSCR(r4)

	/*
	 * Unlock some RTC registers.
	 * (see section 5.11.2 of 860 user's manual)
	 */
	lwi	r3,0x55ccaa33
	stw	r3,RTCSCK(r4)
	stw	r3,RTCK(r4)
	stw	r3,RTSECK(r4)
	stw	r3,RTCALK(r4)

	/*
	 * Clear SERC & ALR. RTC runs on freeze. Enable RTC.
	 */
	li	r3,0x00c3
	sth	r3,RTCSC(r4)

	/*
	 * Clear periodic timer interrupt status.
	 * Enable periodic timer and stop it on freeze.
	 */
        li	r3,0x0083
        sth	r3,PISCR(r4)
	
	/* Clear pending IRQ bits. */
	lwi	r3,0xaaaaaaaa
	stw	r3,SIPEND(r4)

	/* Mask all interrupts. */
	lwi	r3,0
	stw	r3,SIMASK(r4)
	
	/* trigger on falling edge. */
	lwi	r3,0xaaaaaaaa
	stw	r3,SIEL(r4)

	/*
	 * Base Register initialization.
	 */

	/* BOOT ROM */
        lwi	r3,0xfff00801
        lwi	r5,0xfff00140
        stw	r3,BR0(r4)
        stw	r5,OR0(r4)

	/* DRAM and I/O EXPANSION SLOT1 */
        lwi	r3,0x00000001
        lwi	r5,0xfc000008
        stw	r3,BR1(r4)
        stw	r5,OR1(r4)

	/* I/O EXPANSION SLOT2 and SLOT3 */
        lwi	r3,0x04000001
        lwi	r5,0xfc000008
        stw	r3,BR2(r4)
        stw	r5,OR2(r4)

	/* MOTHERBOARD I/O */
        lwi	r3,0x0e000001
        lwi	r5,0xfe000108
        stw	r3,BR3(r4)
        stw	r5,OR3(r4)

	/*
	 *  Read 8-position config switch to determine
	 *  memory settings.
	 */
	lwi	r3,0x0ec00007
	lbz	r3,0(r3)
	andi.	r5,r3,0x30
	bne	1f
	lwi	r1,16*1024*1024
	b	8f
    1:
	cmpwi	r5,0x10
	bne	1f
	lwi	r1,4*1024*1024
	b	8f
    1:
	cmpwi	r5,0x20
	bne	no_dram
	lwi	r1,1*1024*1024
	b	8f
    8:
	andi.	r3,r3,0x80
	bne	1f
	add	r1,r1,r1
    1:
	addi	r3,r1,0
	subi	r1,r1,64
	stw	r3,0(r1)
        blr
no_dram:
	b	no_dram
FUNC_END _bsp_init_stack




