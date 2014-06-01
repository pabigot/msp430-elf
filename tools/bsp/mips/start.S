/*
 * start.S -- MIPS BSP startup.
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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
#ifdef __BOARD_HEADER__
#include __BOARD_HEADER__
#endif
#include <bsp/cpu.h>

	.file 1 "start.S"

	.extern	SYM_NAME(_bsp_init_stack)
	.extern	SYM_NAME(_bsp_init)
	.extern	SYM_NAME(_start)

#ifdef __R3000
#define STORE   sw
#define LOAD    lw
#define MFCOP	mfc0
#define MTCOP	mtc0
#else
#define STORE   sd
#define LOAD    ld
#ifdef __SIM__
#define MFCOP	mfc0
#define MTCOP	mtc0
#else
#define MFCOP	dmfc0
#define MTCOP	dmtc0
#endif
#endif

	.section ".rom_vectors","ax"
	
	.set noreorder
#ifndef __R3000
	.set mips3
#endif

	/*
	 *  Reset, Soft Reset, NMI Vector.
	 *
	 *  This vector is always in ROM.
	 *
	 *  Jump to actual startup code using an uncached address.
	 *  
	 */
	la	k0,SYM_NAME(_start)
	lui	k1,0x2000	/* force to uncached space */
	or	k0,k0,k1
	jr	k0
	 nop

	/*
	 *  For R3000 class processors, the next ROM vector is at
	 *  0xbfc00100. For R4000 class processors, 0xbfc00200.
	 */
#ifdef __R3000
	.p2align 8
#else
	.p2align 9
#endif

__vec_start:
	/*
	 *  TLB Refill Exception. 0xbfc00200
	 */
	la	k1,SYM_NAME(bsp_vsr_table)
#ifdef __R3000
	lw	k1,BSP_EXC_TLB*4(k1)
	 nop
#else
	ld	k1,BSP_EXC_TLB*8(k1)
#endif
	jr	k1
	 li	k0,BSP_EXC_TLB

#ifndef __R3000
	/*
	 *  XTLB Refill Exception.
	 *  0xbfc00280
	 */
	.p2align 7
	la	k1,SYM_NAME(bsp_vsr_table)
	ld	k1,BSP_EXC_XTLB*8(k1)
	jr	k1
	 li	k0,BSP_EXC_XTLB

	/*
	 *  Cache Error Exception.
	 *  0xbfc00300
	 */
	.p2align 7
	la	k1,SYM_NAME(bsp_vsr_table)
	ld	k1,BSP_EXC_CACHE*8(k1)
	jr	k1
	 li	k0,BSP_EXC_CACHE
#endif

	/*
	 *  General Exception Handler.
	 *  mips64:0xbfc00380
	 *  mips32:0xbfc00180
	 */
	.p2align 7
   1:
	mfc0	k0,C0_CAUSE
	la	k1,SYM_NAME(bsp_vsr_table)
	andi	k0,k0,0x1f<<2
	addu	k1,k1,k0
#ifdef __R3000
	lw	k1,0(k1)
#else
	addu	k1,k1,k0
	ld	k1,0(k1)
#endif
	jr	k1
	 srl	k0,k0,2
__vec_end:

#ifdef HAVE_TX_REGS
	/*
	 *  TX-series Debug exception is always in ROM.
	 *  mips64:0xbfc00400
	 *  mips32:0xbfc00200
	 */
	.p2align 7
	la	k1,SYM_NAME(bsp_vsr_table)
#ifdef __R3000
	lw	k1,BSP_EXC_DEBUG*4(k1)
#else
	ld	k1,BSP_EXC_DEBUG*8(k1)
#endif
	jr	k1
	 li	k0,BSP_EXC_DEBUG
#endif /* HAVE_TX_REGS */
	
	

	.text
	.p2align 2
	
FUNC_START _start
#if NMI_SHARES_RESET
	/* check for NMI */
	CHECK_FOR_NMI()
	beqz	k0,1f
	 nop
	la	k1,SYM_NAME(bsp_vsr_table)
#ifdef __R3000
	lw	k1,BSP_EXC_NMI*4(k1)
	 nop
#else
	ld	k1,BSP_EXC_NMI*8(k1)
#endif
	jr	k1
	 li	k0,BSP_EXC_NMI
    1:
#endif
	mtc0	zero,C0_CAUSE	/* reset SW interrupt bits */
	
	li	k0,INIT_SR
	mtc0	k0,C0_STATUS	/* set initial SR */
	
	la	k1,SYM_NAME(_bsp_init_stack)
	lui	k0,0x2000
	or	k1,k1,k0	/* force to uncached space */
	jalr	k1		/* call _bsp_init_stack */
	 nop

	/*
	 *  _bsp_init_stack should have initialized the caches,
	 *  so let's run cached if linked to do so.
	 */
	la	k0,1f
	jr	k0
	 nop
    1:
	/*
	 *  Move vectors into RAM using uncached addresses.
	 */	
        la      a0,__vec_start
        lui     a1,0xa000
        la      a2,__vec_end
    1:
	lw	t1,0(a0)
	addiu	a0,a0,4
	sw	t1,0(a1)
	bne	a0,a2,1b
	 addiu	a1,a1,4

	/*
	 *  Clear SR_BEV so we use the RAM vectors.
	 */
	mfc0	t0,C0_STATUS
	li	t1,SR_BEV
	or	t0,t0,t1
	xor	t0,t0,t1
	mtc0	t0,C0_STATUS

	la	gp,SYM_NAME(_gp)

#if NUM_TLB_ENTRIES
	jal	SYM_NAME(__tlb_init)
	 nop
#endif

	/*
	 * Copy pointer to board-specific RAM info from stack to variable.
	 */
	LOAD	a0,0(sp)
	jal	SYM_NAME(c_rom_start)
	 nop

FUNC_END _start

#ifdef __R3000
#define MASK_UW
#else
#define MASK_UW	 and	k1,k1,t0
#endif
/*
 *  The real work of BSP exception handling is done here.
 *  On entry, k0 contains the BSP exception number.
 */
FUNC_START _bsp_default_handler
	/*
	 *  Make sure to use exception stack.
	 */
	la	k1,_ex_stack
	sltu	k1,k1,sp
	beqz	k1,1f
	 move	k1,sp
	la	sp,_ex_stack
	b	2f
	 nop
    1:
	la	k1,_ex_stack_base
	sltu	k1,sp,k1
	beqz	k1,2f
	 move	k1,sp
	la	sp,_ex_stack
    2:
	STORE	k1,FR_REG29 - EX_STACK_SIZE(sp)	/* save original stack */
	li	k1,EX_STACK_SIZE
#ifdef __R3000
	subu	sp,sp,k1
#else
	dsubu	sp,sp,k1
#endif

	/* save exception number */
	STORE	k0,FR_EXC_NR(sp)

#ifndef __R3000
	lui	k1,0
#endif
	mfc0	k1,C0_STATUS
	STORE	$0,FR_REG0(sp)
	.set	noat
	STORE	$1,FR_REG1(sp)
	.set	at
	STORE	k1,FR_SR(sp)

#ifndef __R3000
	/* Turn off EXL/ERL bits and set kernel mode for mips64 targets */
	ori	k1,k1,31
	xori	k1,k1,31
	mtc0	k1,C0_STATUS
#endif
	MFCOP	k1,C0_BADVA
	STORE	$2,FR_REG2(sp)
	STORE	k1,FR_BAD(sp)
	STORE	$3,FR_REG3(sp)
	STORE	$4,FR_REG4(sp)
	STORE	$5,FR_REG5(sp)
	STORE	$6,FR_REG6(sp)
	STORE	$7,FR_REG7(sp)
	STORE	$8,FR_REG8(sp)
	STORE	$9,FR_REG9(sp)
	STORE	$10,FR_REG10(sp)
	STORE	$11,FR_REG11(sp)
	STORE	$12,FR_REG12(sp)
	STORE	$13,FR_REG13(sp)
	STORE	$14,FR_REG14(sp)
	STORE	$15,FR_REG15(sp)
	STORE	$16,FR_REG16(sp)
	STORE	$17,FR_REG17(sp)
	STORE	$18,FR_REG18(sp)
	STORE	$19,FR_REG19(sp)
	STORE	$20,FR_REG20(sp)
	STORE	$21,FR_REG21(sp)
	STORE	$22,FR_REG22(sp)
	STORE	$23,FR_REG23(sp)
	STORE	$24,FR_REG24(sp)
	STORE	$25,FR_REG25(sp)
	/* no need to save k0/k1 */
	STORE	$28,FR_REG28(sp)
	/* already saved sp */
	STORE	$30,FR_REG30(sp)
	STORE	$30,FR_XFP(sp)
	STORE	$31,FR_REG31(sp)
	mfhi	k1
	STORE	k1,FR_HI(sp)
	mflo	k1
	STORE	k1,FR_LO(sp)
	MFCOP	t0,C0_EPC
	nop
#ifndef __R3000
	STORE	t0,FR_EPC(sp)
	MFCOP	t1,C0_ERRORPC
	STORE	t1,FR_EEPC(sp)
	
	LOAD	k1,FR_SR(sp)
	andi	k1,k1,SR_ERL
	beqz	k1,1f
	 nop
	move	t0,t1
    1:
#endif
	STORE	t0,FR_PC(sp)

#ifndef __R3000
	/* create a mask for following 32-bit cop0 reg saves */
	li	t0,-1
	dsrl32	t0,t0,0
#endif

#ifdef __mips_soft_float
#ifndef __R3000
	sd	$0,FR_FREG0(sp)
	sd	$0,FR_FREG1(sp)
	sd	$0,FR_FREG2(sp)
	sd	$0,FR_FREG3(sp)
	sd	$0,FR_FREG4(sp)
	sd	$0,FR_FREG5(sp)
	sd	$0,FR_FREG6(sp)
	sd	$0,FR_FREG7(sp)
	sd	$0,FR_FREG8(sp)
	sd	$0,FR_FREG9(sp)
	sd	$0,FR_FREG10(sp)
	sd	$0,FR_FREG11(sp)
	sd	$0,FR_FREG12(sp)
	sd	$0,FR_FREG13(sp)
	sd	$0,FR_FREG14(sp)
	sd	$0,FR_FREG15(sp)
	sd	$0,FR_FREG16(sp)
	sd	$0,FR_FREG17(sp)
	sd	$0,FR_FREG18(sp)
	sd	$0,FR_FREG19(sp)
	sd	$0,FR_FREG20(sp)
	sd	$0,FR_FREG21(sp)
	sd	$0,FR_FREG22(sp)
	sd	$0,FR_FREG23(sp)
	sd	$0,FR_FREG24(sp)
	sd	$0,FR_FREG25(sp)
	sd	$0,FR_FREG26(sp)
	sd	$0,FR_FREG27(sp)
	sd	$0,FR_FREG28(sp)
	sd	$0,FR_FREG29(sp)
	sd	$0,FR_FREG30(sp)
	sd	$0,FR_FREG31(sp)
#else
	sw	$0,FR_FREG0(sp)
	sw	$0,FR_FREG1(sp)
	sw	$0,FR_FREG2(sp)
	sw	$0,FR_FREG3(sp)
	sw	$0,FR_FREG4(sp)
	sw	$0,FR_FREG5(sp)
	sw	$0,FR_FREG6(sp)
	sw	$0,FR_FREG7(sp)
	sw	$0,FR_FREG8(sp)
	sw	$0,FR_FREG9(sp)
	sw	$0,FR_FREG10(sp)
	sw	$0,FR_FREG11(sp)
	sw	$0,FR_FREG12(sp)
	sw	$0,FR_FREG13(sp)
	sw	$0,FR_FREG14(sp)
	sw	$0,FR_FREG15(sp)
	sw	$0,FR_FREG16(sp)
	sw	$0,FR_FREG17(sp)
	sw	$0,FR_FREG18(sp)
	sw	$0,FR_FREG19(sp)
	sw	$0,FR_FREG20(sp)
	sw	$0,FR_FREG21(sp)
	sw	$0,FR_FREG22(sp)
	sw	$0,FR_FREG23(sp)
	sw	$0,FR_FREG24(sp)
	sw	$0,FR_FREG25(sp)
	sw	$0,FR_FREG26(sp)
	sw	$0,FR_FREG27(sp)
	sw	$0,FR_FREG28(sp)
	sw	$0,FR_FREG29(sp)
	sw	$0,FR_FREG30(sp)
	sw	$0,FR_FREG31(sp)
#endif	
#else
	/* hardware float */
	mfc0	k1,C0_STATUS
	sll	t0,k1,2         /* use CU1 bit as sign bit */
	bgez	t0,3f		/* branch if FPU disabled */
	 nop
#ifndef __R3000
	sll	t0,k1,5		/* use FR bit as sign bit */
	bgez	t0,1f		/* branch if FPU in 32-bit mode */
	 nop
	sdc1	$0,FR_FREG0(sp)
	sdc1	$1,FR_FREG1(sp)
	sdc1	$2,FR_FREG2(sp)
	sdc1	$3,FR_FREG3(sp)
	sdc1	$4,FR_FREG4(sp)
	sdc1	$5,FR_FREG5(sp)
	sdc1	$6,FR_FREG6(sp)
	sdc1	$7,FR_FREG7(sp)
	sdc1	$8,FR_FREG8(sp)
	sdc1	$9,FR_FREG9(sp)
	sdc1	$10,FR_FREG10(sp)
	sdc1	$11,FR_FREG11(sp)
	sdc1	$12,FR_FREG12(sp)
	sdc1	$13,FR_FREG13(sp)
	sdc1	$14,FR_FREG14(sp)
	sdc1	$15,FR_FREG15(sp)
	sdc1	$16,FR_FREG16(sp)
	sdc1	$17,FR_FREG17(sp)
	sdc1	$18,FR_FREG18(sp)
	sdc1	$19,FR_FREG19(sp)
	sdc1	$20,FR_FREG20(sp)
	sdc1	$21,FR_FREG21(sp)
	sdc1	$22,FR_FREG22(sp)
	sdc1	$23,FR_FREG23(sp)
	sdc1	$24,FR_FREG24(sp)
	sdc1	$25,FR_FREG25(sp)
	sdc1	$26,FR_FREG26(sp)
	sdc1	$27,FR_FREG27(sp)
	sdc1	$28,FR_FREG28(sp)
	sdc1	$29,FR_FREG29(sp)
	sdc1	$30,FR_FREG30(sp)
	sdc1	$31,FR_FREG31(sp)
	b	2f
	 nop
    1:
#endif
	swc1	$0,FR_FREG0(sp)
	swc1	$1,FR_FREG1(sp)
	swc1	$2,FR_FREG2(sp)
	swc1	$3,FR_FREG3(sp)
	swc1	$4,FR_FREG4(sp)
	swc1	$5,FR_FREG5(sp)
	swc1	$6,FR_FREG6(sp)
	swc1	$7,FR_FREG7(sp)
	swc1	$8,FR_FREG8(sp)
	swc1	$9,FR_FREG9(sp)
	swc1	$10,FR_FREG10(sp)
	swc1	$11,FR_FREG11(sp)
	swc1	$12,FR_FREG12(sp)
	swc1	$13,FR_FREG13(sp)
	swc1	$14,FR_FREG14(sp)
	swc1	$15,FR_FREG15(sp)
	swc1	$16,FR_FREG16(sp)
	swc1	$17,FR_FREG17(sp)
	swc1	$18,FR_FREG18(sp)
	swc1	$19,FR_FREG19(sp)
	swc1	$20,FR_FREG20(sp)
	swc1	$21,FR_FREG21(sp)
	swc1	$22,FR_FREG22(sp)
	swc1	$23,FR_FREG23(sp)
	swc1	$24,FR_FREG24(sp)
	swc1	$25,FR_FREG25(sp)
	swc1	$26,FR_FREG26(sp)
	swc1	$27,FR_FREG27(sp)
	swc1	$28,FR_FREG28(sp)
	swc1	$29,FR_FREG29(sp)
	swc1	$30,FR_FREG30(sp)
	swc1	$31,FR_FREG31(sp)
    2:	
	mfc1	k1,C1_STATUS
	STORE	k1,FR_FSR(sp)
	mfc1	k1,C1_REVISION
	STORE	k1,FR_FIR(sp)
    3:
#endif

	move	a0,k0		/* BSP exception number */
#ifndef __R3000
	lui	k0,0
#endif
	mfc0	k0,C0_CAUSE
	move	a1,sp		/* saved regs pointer */
	STORE	k0,FR_CAUSE(sp)

	/* setup stub gp pointer. */
	la	gp,SYM_NAME(_gp)
	
	jal	SYM_NAME(_bsp_exc_dispatch)
	 move	s0,a0

	/*
	 * Save return value so we can invoke the debug agent if necessary after
	 * restoring registers.
	 */
	move	k0,v0

#ifndef __mips_soft_float
	mfc0	k1,C0_STATUS
	sll	t0,k1,2         /* use CU1 bit as sign bit */
	bgez	t0,3f		/* branch if FPU disabled */
	 nop
#ifndef __R3000
	sll	t0,k1,5		/* use FR bit as sign bit */
	bgez	t0,1f		/* branch if FPU in 32-bit mode */
	 nop
	ldc1	$31,FR_FREG31(sp)
	ldc1	$30,FR_FREG30(sp)
	ldc1	$29,FR_FREG29(sp)
	ldc1	$28,FR_FREG28(sp)
	ldc1	$27,FR_FREG27(sp)
	ldc1	$26,FR_FREG26(sp)
	ldc1	$25,FR_FREG25(sp)
	ldc1	$24,FR_FREG24(sp)
	ldc1	$23,FR_FREG23(sp)
	ldc1	$22,FR_FREG22(sp)
	ldc1	$21,FR_FREG21(sp)
	ldc1	$20,FR_FREG20(sp)
	ldc1	$19,FR_FREG19(sp)
	ldc1	$18,FR_FREG18(sp)
	ldc1	$17,FR_FREG17(sp)
	ldc1	$16,FR_FREG16(sp)
	ldc1	$15,FR_FREG15(sp)
	ldc1	$14,FR_FREG14(sp)
	ldc1	$13,FR_FREG13(sp)
	ldc1	$12,FR_FREG12(sp)
	ldc1	$11,FR_FREG11(sp)
	ldc1	$10,FR_FREG10(sp)
	ldc1	$9,FR_FREG9(sp)
	ldc1	$8,FR_FREG8(sp)
	ldc1	$7,FR_FREG7(sp)
	ldc1	$6,FR_FREG6(sp)
	ldc1	$5,FR_FREG5(sp)
	ldc1	$4,FR_FREG4(sp)
	ldc1	$3,FR_FREG3(sp)
	ldc1	$2,FR_FREG2(sp)
	ldc1	$1,FR_FREG1(sp)
	ldc1	$0,FR_FREG0(sp)
	b	2f
	 nop
    1:
#endif
	lwc1	$31,FR_FREG31(sp)
	lwc1	$30,FR_FREG30(sp)
	lwc1	$29,FR_FREG29(sp)
	lwc1	$28,FR_FREG28(sp)
	lwc1	$27,FR_FREG27(sp)
	lwc1	$26,FR_FREG26(sp)
	lwc1	$25,FR_FREG25(sp)
	lwc1	$24,FR_FREG24(sp)
	lwc1	$23,FR_FREG23(sp)
	lwc1	$22,FR_FREG22(sp)
	lwc1	$21,FR_FREG21(sp)
	lwc1	$20,FR_FREG20(sp)
	lwc1	$19,FR_FREG19(sp)
	lwc1	$18,FR_FREG18(sp)
	lwc1	$17,FR_FREG17(sp)
	lwc1	$16,FR_FREG16(sp)
	lwc1	$15,FR_FREG15(sp)
	lwc1	$14,FR_FREG14(sp)
	lwc1	$13,FR_FREG13(sp)
	lwc1	$12,FR_FREG12(sp)
	lwc1	$11,FR_FREG11(sp)
	lwc1	$10,FR_FREG10(sp)
	lwc1	$9,FR_FREG9(sp)
	lwc1	$8,FR_FREG8(sp)
	lwc1	$7,FR_FREG7(sp)
	lwc1	$6,FR_FREG6(sp)
	lwc1	$5,FR_FREG5(sp)
	lwc1	$4,FR_FREG4(sp)
	lwc1	$3,FR_FREG3(sp)
	lwc1	$2,FR_FREG2(sp)
	lwc1	$1,FR_FREG1(sp)
	lwc1	$0,FR_FREG0(sp)
    2:	
	LOAD	k1,FR_FSR(sp)
	 nop
	mtc1	k1,C1_STATUS
    3:
#endif
	LOAD	v0,FR_HI(sp)
	LOAD	k1,FR_LO(sp)
	mthi	v0
	LOAD	v0,FR_SR(sp)
	mtlo	k1
	mtc0	v0,C0_STATUS
	LOAD	$31,FR_REG31(sp)
#ifndef __R3000
	LOAD	k1,FR_EEPC(sp)
	MTCOP	k1,C0_ERRORPC
	LOAD	k1,FR_EPC(sp)
	MTCOP	k1,C0_EPC
#endif	
	LOAD	$30,FR_REG30(sp)
	/* sp gets restored last */
	LOAD	$28,FR_REG28(sp)
	/* no need to restore k0/k1 */
	LOAD	$25,FR_REG25(sp)
	LOAD	$24,FR_REG24(sp)
	LOAD	$23,FR_REG23(sp)
	LOAD	$22,FR_REG22(sp)
	LOAD	$21,FR_REG21(sp)
	LOAD	$20,FR_REG20(sp)
	LOAD	$19,FR_REG19(sp)
	LOAD	$18,FR_REG18(sp)
	LOAD	$17,FR_REG17(sp)
	LOAD	$16,FR_REG16(sp)
	LOAD	$15,FR_REG15(sp)
	LOAD	$14,FR_REG14(sp)
	LOAD	$13,FR_REG13(sp)
	LOAD	$12,FR_REG12(sp)
	LOAD	$11,FR_REG11(sp)
	LOAD	$10,FR_REG10(sp)
	LOAD	$9,FR_REG9(sp)
	LOAD	$8,FR_REG8(sp)
	LOAD	$7,FR_REG7(sp)
	LOAD	$6,FR_REG6(sp)
	LOAD	$5,FR_REG5(sp)
	LOAD	$4,FR_REG4(sp)
	LOAD	$3,FR_REG3(sp)
	LOAD	$2,FR_REG2(sp)
	.set	noat
	LOAD	$1,FR_REG1(sp)
	.set	at
	
	/*
	 * Now check return value from _bsp_exc_dispatch to see if we
	 * invoke debug agent or return.
	 */
	bnez	k0,1f
	 nop

	/* invoke debug agent */
	la	k1,SYM_NAME(bsp_vsr_table)
#ifdef __R3000
	lw	k1,BSP_VEC_STUB_ENTRY*4(k1)
#else
	ld	k1,BSP_VEC_STUB_ENTRY*8(k1)
#endif
	LOAD	k0,FR_EXC_NR(sp)
	LOAD	sp,FR_REG29(sp)
	jr	k1
	 nop
	/* never returns */
    1:
#ifdef __R3000
	LOAD	k1,FR_PC(sp)
	LOAD	sp,FR_REG29(sp)
	jr	k1
	 rfe
#else
	LOAD	sp,FR_REG29(sp)
	eret
	 nop
#endif

FUNC_END _bsp_default_handler


	.section ".ram_vectors","ax"
	.globl SYM_NAME(bsp_vsr_table)
  SYM_NAME(bsp_vsr_table):
	.rept NUM_VTAB_ENTRIES
#ifdef __R3000
	.long 0
#else
	.quad 0
#endif
	.endr	
	
	
	/*
	 *  Provide a stack for use exceptions and interrupts.
	 *  Note that this stack is in .bss, so it will be below
	 *  the normal user stack which is placed in high memory.
	 */
	.section ".bss","a"
	.p2align 4
	.globl _ex_stack_base
  _ex_stack_base:
	.rept	2048
	.long	0
	.endr
	.globl _ex_stack
  _ex_stack:	

