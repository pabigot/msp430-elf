/*
 * init_yka.S -- Yellowknife specific initialization.
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

	.file "init_ykn.S"

#include <bsp/cpu.h>
#include "yellowknife.h"
	
	.text
        .align     2            # align on word boundary


	/*
	 * Write to PCI config register.
	 * Assumes r5 => CONFIG_ADDR_REG
	 *         r6 => CONFIG_DATA_REG
	 *         r0 => zero
	 */
	.macro pcicfg_write addr val
        lwi     r3,\addr
        stwbrx  r3,r0,r5
        lwi	r4,\val
        stwbrx  r4,r0,r6
        sync
	lwbrx	r4,r0,r6
	sync
	.endm
	
	
FUNC_START _bsp_init_stack
	lis	r0,0

	/* MPC106 PCI ID */
	lis	r4,MPC106_DEVICE_ID
	ori	r4,r4,MPC106_VENDOR_ID

	/* Address of MPC106 ID registers */
	lwi	r3,MPC106_CFG_ID
	
#if defined(__MPC106_MAP_A__)
	/* check for PREP mode */
	lwi	r5,PREP_CFG_ADDR
	lwi	r6,PREP_CFG_DATA
	lwi	r7,PREP_IO_BASE
	lwi	r8,PREP_ADDR_MAP
#elif defined(__MPC106_MAP_B__)
	lwi	r5,CHRP_CFG_ADDR
	lwi	r6,CHRP_CFG_DATA
	lwi	r7,CHRP_IO_BASE
	lwi	r8,CHRP_ADDR_MAP
#else
#error Must define Memory Map
#endif

    1:
	/*
	 * Find out if we are X2 or X4.
	 * X2 ROM is on CPU bus. X4 ROM is on PCI bus.
	 */
	lwi	r3,MPC106_CFG_PICR1
	stwbrx	r3,r0,r5
	sync
	lwbrx	r3,r0,r6
	sync
	andis.	r3,r3,0x10	/* RCS0 bit */
	bne	x2		/* x2 if bit not set */
	
	/* x4 */
	lwi	r4,0x3f041800
	or	r4,r4,r8	/* address map */
	stwbrx	r4,r0,r6
	sync
	lwbrx	r4,r0,r6
	sync

	pcicfg_write MPC106_CFG_PICR2  0x04470215
	pcicfg_write MPC106_CFG_MCCR1  0xffa4ffff
	pcicfg_write MPC106_CFG_MCCR2  0x00000c34
	pcicfg_write MPC106_CFG_MCCR3  0xf8300000
	pcicfg_write MPC106_CFG_MCCR4  0x25302244
	pcicfg_write MPC106_CFG_MSAR1  0xffffff00
	pcicfg_write MPC106_CFG_MSAR2  0xffffffff
	pcicfg_write MPC106_CFG_MESAR1 0x03030300
	pcicfg_write MPC106_CFG_MESAR2 0x03030303
	pcicfg_write MPC106_CFG_MEAR1  0xffffff10
	pcicfg_write MPC106_CFG_MEAR2  0xffffffff
	pcicfg_write MPC106_CFG_MEEAR1 0x03030300
	pcicfg_write MPC106_CFG_MEEAR2 0x03030303
	pcicfg_write MPC106_CFG_MBEN   0x00080001
	
	/* Wait before initializing other regs */
	li	r3,0x3800
	mtctr   r3
    1:
	bdnz	1b
	
	lwi	r3,MPC106_CFG_MCCR1
	stwbrx	r3,r0,r5
	sync
	lwbrx	r4,0,r6
	lis	r3,0x0008
	or	r4,r4,r3
	stwbrx	r4,r0,r6
	sync
	lwbrx	r4,r0,r6

	/* Again with the waiting */
	li	r3,8000
	mtctr	r3
    1:
	bdnz	1b

	sync
	b	9f

    x2:
	pcicfg_write MPC106_CFG_MSAR1 0x00000000
	pcicfg_write MPC106_CFG_MEAR1 0x00000007
	
	/* RAM enable */
	lwi	r3,MPC106_CFG_MBEN
	li	r4,1
	stwbrx	r3,r0,r5
	stb	r4,0(r6)
	sync

	lwi	r3,MPC106_CFG_PICR1
	lwi	r4,0xff900a18
	or	r4,r4,r8	/* address map */
        stwbrx  r3,r0,r5
        stwbrx  r4,r0,r6
        sync

	pcicfg_write MPC106_CFG_PICR2 0x000C060C
	pcicfg_write MPC106_CFG_MCCR2 0x00000707
	pcicfg_write MPC106_CFG_MCCR3 0x0002d314
	pcicfg_write MPC106_CFG_MCCR4 0x00000000
	pcicfg_write MPC106_CFG_MCCR1 0xff8a0001

    9:
	/* setup mmu */
	lwi	r4,0xfff00012
	lwi	r3,0xfff001ff
	isync
	mtspr	ibat0l,r4
	isync
	mtspr	ibat0u,r3
	isync
	mtspr	dbat0l,r4
	isync
	mtspr	dbat0u,r3
	isync
	sync

	/* local memory mapping */
	li	r4,0x12
	li	r3,0xfff
	isync
	mtspr	ibat1l,r4
	isync
	mtspr	ibat1u,r3
	isync
	mtspr	dbat1l,r4
	isync
	mtspr	dbat1u,r3
	isync
	sync

	/* 105/106, UMC/Winbond, and SIO mapping */
        ori     r4,r7,0x0022
        ori     r3,r7,0x01ff
	isync
	mtspr	ibat2l,r4
	isync
	mtspr	ibat2u,r3
	isync
	mtspr	dbat2l,r4
	isync
	mtspr	dbat2u,r3
	isync
	sync

	/* bat3 is our Floating BAT */
	isync
	mtspr	ibat3l,r0
	isync
	mtspr	ibat3u,r0
	isync
	mtspr	dbat3l,r0
	isync
	mtspr	dbat3u,r0
	isync
        sync

	/* init tlb */
	li	r3,0
	mfspr	r4,pvr
	srawi	r4,r4,16

	cmpwi	r4,3
	beq	1f
	cmpwi	r4,6
	beq	1f
	cmpwi	r4,7
	beq	1f

	lis	r5,4
	b	2f
    1:
	lis	r5,2
    2:	
	tlbie	r3
	sync
	addi	r3,r3,0x1000
	cmpw	r3,r5
	blt	2b

	sync

	/* size RAM */
	lwi	r3,0		/* start address      */
	lwi	r4,0x08000000	/* end address (128MB) */
	lwi	r6,0x00100000	/* 1MB increment      */

	/* store pattern into memory */
	lwi	r5,0x12345678
    1:
	stw	r5,0(r3)
	add	r3,r3,r6
	cmpw	r3,r4
	bne	1b
	
	/* now, check data, filling behind with zero */
	lwi	r3,0		/* start address */
    1:
	lwz	r7,0(r3)
	cmpw	r7,r5
	bne	2f
	stw	r0,0(r3)			
	add	r3,r3,r6
	cmpw	r3,r4
	bne	1b
    2:
	/* r3 points just past end of memory */
	/* save top of mem to stack for later copy */
#if 0
	lwi	r3, (4 * 1024 * 1024)
#endif
	subi	r1,r3,64
#if defined(__SWITCH_TO_LE__) && !defined(__LITTLE_ENDIAN__)
	/* 
	 * Need to munge the address if we're storing big-endian and
	 * later reading little-endian
	 */
        stw	r3,4(r1)
#else
	stw	r3,0(r1)
#endif

	/* turn on data translation */
	mfmsr	r3
	ori	r3,r3,0x0030
	mtmsr	r3
	isync
	sync
#if defined(__SWITCH_TO_LE__) && !defined(__LITTLE_ENDIAN__)
	/* Copy switch code from ROM to RAM */
	lwi	r3,SYM_NAME(__bsp_le_switch)
	lwi	r4,0x4000
	lwi	r5,SYM_NAME(.L__bsp_le_switch)
1:
	lwz	r0,0(r3)
	stw	r0,0(r4)
	addi	r3,r3,4
	addi	r4,r4,4
	cmplw	r3,r5
	blt	1b
2:
	lwi	r4,0x4000
	mtctr	r4
	bctr
#else
        blr
#endif
FUNC_END _bsp_init_stack



#if defined(__SWITCH_TO_LE__) && !defined(__LITTLE_ENDIAN__)
/*
 * We move this to RAM before executing so it needs to be position independent.
 */
FUNC_START __bsp_le_switch
#if defined(__MPC106_MAP_A__)
	/* check for PREP mode */
	lwi	r5,PREP_CFG_ADDR
	lwi	r6,PREP_CFG_DATA
#elif defined(__MPC106_MAP_B__)
	lwi	r5,CHRP_CFG_ADDR
	lwi	r6,CHRP_CFG_DATA
#endif
	xor	r0,r0,r0	
	lwi	r3,MPC106_CFG_PICR1
	stwbrx	r3,r0,r5
	eieio
	sync
	lwbrx	r3,r0,r6
	ori	r3,r3,0x20  /* LE_MODE */
	stwbrx	r3,r0,r6
	eieio
	sync

	mfmsr	r4
	lwi	r3,MSR_ILE
	or	r4,r4,r3
	mtmsr	r4
	lwi	r3,MSR_LE
	or	r4,r4,r3
	mtspr	srr1,r4
	mflr	r3
	mtspr	srr0,r3
	sync
	rfi
FUNC_END __bsp_le_switch
#endif
