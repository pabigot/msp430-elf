/*
 * init_mbx.S -- MBX specific initialization.
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
	.file "init_mbx.S"
	.text

#include <bsp/cpu.h>
	
        .align     2
	
#define __40MHZ 1
	
FUNC_START _bsp_init_stack
	
#ifndef USE_MONITOR
	/*
	 * Set base of internal map.
	 */
	lis	r4,0xfa20
	mtspr	immr,r4

	/*
	 * SIU Initialization.
	 */
/*        lwi	r3,0x0062A900*/
/*        lwi	r3,0x006A2900*/
	lwi	r3,0x00602400
        stw	r3,SIUMCR(r4)

	/*
	 * Enable bus monitor. Disable Watchdog timer.
	 */
	lwi	r3,0xffffff88
	stw	r3,SYPCR(r4)

	/*
	 * Clear REFA & REFB. Enable but freeze timebase.
	 */
	lwi	r3,0x00c2
	sth	r3,TBSCR(r4)

	/*
	 * Unlock some RTC registers (see section 5.11.2)
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
	
	/*
	 * Perform UPM programming by writing to its 64 RAM locations.
	 * Note that UPM initialization must be done before the Bank Register
	 * initialization. Otherwise, system may hang when writing to Bank
	 * Registers in certain cases.
	 */
	lis	r5,__upmtbl_start@h
	ori	r5,r5,__upmtbl_start@l
	lis	r6,__upmtbl_end@h
	ori	r6,r6,__upmtbl_end@l
	sub	r7,r6,r5      /* size of table */ 
	srawi	r7,r7,2       /* in words */
        
        li	r6,0x0000     /* Command - OP=Write, UPMA, MAD=0 */
    1:
        lwz	r3,0(r5)      /* get data from table */
        stw	r3,MDR(r4)    /* store the data to MD register */
        stw	r6,MCR(r4)    /* issue command to MCR register */
        addi	r5,r5,4       /* next entry in the table */
        addi	r6,r6,1       /* next MAD address */
        cmpw	r6,r7         /* done yet ? */
        blt	1b

	/*
	 * Set refresh timer prescaler to divide by 8.
	 */
        li	r3,PTP_DIV32
        sth	r3,MPTPR(r4)

        /*
	 * See Table 15-16 MPC860 User's Manual.
	 *
	 * PTA = 0x13 (clock dependent)
	 * PTAE = enabled
	 * AMA = 0
	 * DSA = 2 cycle disable
	 * G0CLA = A12
	 * GPL_A4DIS = 0
	 * RLFA = 0
	 * WLFA = 0
	 * TLFA = 0
	 */
	lwi	r3,0x13821000
        stw	r3,MAMR(r4)

	/*
	 * Base Register initialization.
	 */
	/* BOOT ROM */
        lwi	r3,0xfe000401
        lwi	r5,0xff800930
        stw	r3,BR0(r4)
        stw	r5,OR0(r4)

	/* ONBOARD DRAM */
        lwi	r3,0x00000081
        lwi	r5,0xffc00400
        stw	r3,BR1(r4)
        stw	r5,OR1(r4)

	/* DRAM DIMM BANK0 */
        lwi	r3,0x00000080
        lwi	r5,0x00000400
        stw	r3,BR2(r4)
        stw	r5,OR2(r4)

	/* DRAM DIMM BANK1 */
        lwi	r3,0x00000080
        lwi	r5,0x00000400
        stw	r3,BR3(r4)
        stw	r5,OR3(r4)

	/* NVRAM */
        lwi	r3,0xfa000401
        lwi	r5,0xffe00920
        stw	r3,BR4(r4)
        stw	r5,OR4(r4)

	/* PCI BRIDGE MEM/IO */
        lwi	r3,0x80000001
        lwi	r5,0xa0000108
        stw	r3,BR5(r4)
        stw	r5,OR5(r4)

	/* PCI BRIDGE REGISTERS */
        lwi	r3,0xfa210001
        lwi	r5,0xffff0108
        stw	r3,BR6(r4)
        stw	r5,OR6(r4)

	/* FLASH */
        lwi	r3,0xfc000001
        lwi	r5,0xff800930
        stw	r3,BR7(r4)
        stw	r5,OR7(r4)
	
	/*
	 *  Turn off MBX LEDs
	 */
	lwi	r3,0xfa100001
	li	r5,4
	stb	r5,0(r3)

	/*
	 *  SYSTEM CLOCK CONTROL REGISTER
	 *
	 *  COM   (1:2)   = 0
	 *  TBS   (6)     = 1
	 *  RTDIV (7)     = 0
	 *  RTSEL (8)     = 0
	 *  CRQEN (9)     = 0
	 *  PRQEN (10)    = 0
	 *  EBDF  (13:14) = 0
	 *  DFSYNC(17:18) = 0
	 *  DFBRG (19:20) = 0
	 *  DFNL  (21:23) = 0
	 *  DFNH  (24:26) = 0
	 */
	lwi	r3,0x02000000
	stw	r3,SCCR(r4)
	
	/*
	 *  The following sets up a 40MHz CPU clock.
	 *  I've seen 2 variations of MBX boards. One
	 *  uses a direct feed (1:1) 40MHz clock on
	 *  EXTCLK inputs. The other uses a 32KHz
	 *  oscillator on the OSCM inputs.
	 */
	lwz	r3,PLPRCR(r4)
	rlwinm	r3,r3,12,20,31
	cmpwi	r3,0
	beq     1f
	
	/*
	 *  PLL, LOW POWER, AND RESET CONTROL REGISTER
	 *
	 *  MF    (0:11)  = depends on source clock
	 *  SPLSS (16)    = 1
	 *  TEXPS (17)    = 1
	 *  TMIST (19)    = 1
	 *  CSRC  (21)    = 0
	 *  LPM   (22:23) = 0
	 *  CSR   (24)    = 0
	 *  LOLRE (25)    = 0
	 *  FIOPD (26)    = 0
	 */
	/*  MF    (0:11)  = 0x4c4 = 1220 = (40MHz/32.768KHz) */
	lwi	r3,0x4c40d000
	b	2f
    1:
	/*  MF    (0:11)  = 0x000 = 1 = (1:1) */
	lwi	r3,0x0000d000
    2:
	stw	r3,PLPRCR(r4)

	/*
	 *  Turn off MBX LEDs
	 */
	lwi	r3,0xfa100001
	li	r5,6
	stb	r5,0(r3)
	
	/* give us a stack
	 * At this point, we should size DRAM.
	 */
	lwi	r3,0x00400000
#else
	lwi	r3,0x00200000
#endif

	subi	r1,r3,64

	/*
	 * save top of mem to stack for later copy
	 */
	stw	r3,0(r1)
	
        blr
FUNC_END _bsp_init_stack

__upmtbl_start:

#ifdef __25MHZ
	/* UPM contents for 25MHz clk. DRAM: EDO,4K,60ns */
	
	/* Single read. (offset 0 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst read. (offset 8 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0xffffec07, 0xffffec07
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Single write. (offset 18 in upm RAM) */
        .long	0xcfffe004, 0x0fffa404, 0x08ff2c00, 0x33ff6c0f
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst write. (offset 20 in upm RAM) */
        .long	0xcfffe004, 0x0fffa404, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x03ff2c0c, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x33ff6c0f, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Refresh (offset 30 in upm RAM) */
        .long	0xc0ffec04, 0x07ffec04, 0x3fffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	
	/* Exception. (offset 3c in upm RAM) */
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
#endif

#ifdef __33MHZ
	/* UPM contents for 33MHz clk. DRAM: EDO,4K,60ns */
	
	/* Single read. (offset 0 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst read. (offset 8 in upm RAM) */
        .long	0xcfffe004, 0x0fffe404, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0x08af2c04, 0x03af2c08
        .long	0x08af2c04, 0x03af2c08, 0xffffec07, 0xffffec07
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Single write. (offset 18 in upm RAM) */
        .long	0xcfffe004, 0x0fff2404, 0x08ff2c00, 0x33ff6c07
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst write. (offset 20 in upm RAM) */
        .long	0xcfffe004, 0x0fff2404, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x03ff2c0c, 0x08ff2c00, 0x03ff2c0c
        .long	0x08ff2c00, 0x33ff6c07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Refresh (offset 30 in upm RAM) */
        .long	0xc0ffec04, 0x03ffec04, 0x1fffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	
	/* Exception. (offset 3c in upm RAM) */
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
#endif

#ifdef __40MHZ
	/* UPM contents for 40MHz clk. DRAM: EDO,4K,60ns */
	
	/* Single read. (offset 0 in upm RAM) */
        .long	0xefffe004, 0x0fffe004, 0x0eefac04, 0x00af2c04
        .long	0x03af2c08, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst read. (offset 8 in upm RAM) */
        .long	0xefffe004, 0x0fffe004, 0x0eefac04, 0x00af2c04
        .long	0x03af2c08, 0x0caf2c04, 0x00af2c04, 0x03af2c08
        .long	0x0caf2c04, 0x00af2c04, 0x03af2c08, 0x0caf2c04
        .long	0x00af2c04, 0x03af2c08, 0xffffec07, 0xffffec07

	/* Single write. (offset 18 in upm RAM) */
        .long	0xefffe004, 0x0fffa004, 0x0eff2c04, 0x00ff2c00
        .long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Burst write. (offset 20 in upm RAM) */
        .long	0xefffe004, 0x0fffa004, 0x0eff2c04, 0x00ff2c00
        .long	0x0fff2c0c, 0x0cff2c00, 0x03ff2c0c, 0x0cff2c00
        .long	0x03ff2c0c, 0x0cff2c00, 0x33ff6c07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07

	/* Refresh (offset 30 in upm RAM) */
        .long	0xf0ffec04, 0x00ffec04, 0x0fffec04, 0x0fffec04
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
	
	/* Exception. (offset 3c in upm RAM) */
	.long	0xffffec07, 0xffffec07, 0xffffec07, 0xffffec07
#endif

__upmtbl_end:

FUNC_START set_led
	lwi	r4,0xfa100001
	stb	r3,0(r4)
	blr
FUNC_END set_led
	
FUNC_START flash_led
	lwi	r4,0xfa100001
    1:	
	li	r3,4
	stb	r3,0(r4)
	
	lis	r5,10
	mtctr   r5
    2:	
	bdnz	2b

	li	r3,2
	stb	r3,0(r4)
	
	lis	r5,10
	mtctr   r5
    3:	
	bdnz	3b
	b 1b
FUNC_END flash_led

	
