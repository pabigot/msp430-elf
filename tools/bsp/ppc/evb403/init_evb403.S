/*
 * init_evb403.S -- Initialization for IBM EVB 403 board.
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
	.file "init_evb403.S"

#include <bsp/cpu.h>

	.section ".ppc_reset_403","ax"
	ba	SYM_NAME(__init_403)

	.section ".ppc_init_403","ax"
SYM_NAME(__init_403):
	xor	r0,r0,r0
	mtiocr	r0
	mtexier	r0
	isync
	lwi	r3,0xff180242
	mtbr0	r3
	isync
	lwi	r3,0xe0180468
	mtbr1	r3
	isync
	lwi	r3,0x401846f4
	mtbr2	r3
	isync
	lwi	r3,0x0878c200
	mtbr3	r3
	isync
	lwi	r3,0xff00bfff
	mtbr4	r3
	isync
	lwi	r3,0xff00bfff
	mtbr5	r3
	isync
	lwi	r3,0xff00bfff
	mtbr6	r3
	isync
	lwi	r3,0x00590a8e
	mtbr7	r3
	isync
	b	SYM_NAME(start)	

	.text
        .align     2

FUNC_START _bsp_init_stack

	li	r3,32
	mtctr	r3
	xor	r3,r3,r3
    1:
	dccci	r0,r3
	addi	r3,r3,16
	bdnz	1b
	li	r3,64
	mtctr	r3
	xor	r3,r3,r3
    1:
	iccci	r0,r3
	addi	r3,r3,16
	bdnz	1b
	xor	r3,r3,r3
	mticcr	r3
	mtdccr	r3

	mtdmacr0 r3
	mtdmacr1 r3
	mtdmacr2 r3
	mtdmacr3 r3
	isync

	mtbesr	r3
	mtesr	r3
	mtxer	r3

	mfiocr	r4
	lwi	r3,0xfffffffb
	and	r4,r4,r3
	mtiocr	r4
	
	xor	r3,r3,r3
	mttcr	r3
	
	lwi	r3,0xfffbffff
	mfmsr	r4
	and	r5,r4,r3
	mtmsr	r5
	isync

	xor	r3,r3,r3
	mtspr	tblw,r3
	li	r4,4096
	mtspr	tbuw,r4

	lwi	r3,0x00400000
	subi	r1,r3,64

	/*
	 * save top of mem to stack for later copy
	 */
	stw	r3,0(r1)
	blr
FUNC_END _bsp_init_stack
