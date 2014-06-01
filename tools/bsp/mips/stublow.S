/*
 * stublow.S -- MIPS BSP stub entry.
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
#include "syscall.h"
	
	.file 1 "stublow.S"

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

	.text
	.p2align 2
	.set noreorder

#ifdef __R3000
#define MASK_UW
#else
	.set mips3
#define MASK_UW	 and	k1,k1,t0
#endif


/*
 *  This is the where exceptions wind up if they need to have the debug
 *  agent take control.
 *  On entry:
 *    k0 = BSP exception number.
 *    k1 = undefined
 *    All other registers have immediate post-exception values.
 */
FUNC_START _bsp_stub_entry
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
#if defined(FR_INDEX)
	mfc0	k1,C0_INDEX
	MASK_UW
	STORE	k1,FR_INDEX(sp)
#endif
#if defined(FR_CONTEXT)
	MFCOP	k1,C0_CONTEXT
	STORE	k1,FR_CONTEXT(sp)
#endif
#if defined(FR_XCONTEXT)
	MFCOP	k1,C0_XCONTEXT
	STORE	k1,FR_XCONTEXT(sp)
#endif
#if defined(FR_PAGEMASK)
	mfc0	k1,C0_PAGEMASK
	MASK_UW
	STORE	k1,FR_PAGEMASK(sp)
#endif
#if defined(FR_WIRED)
	mfc0	k1,C0_WIRED
	MASK_UW
	STORE	k1,FR_WIRED(sp)
#endif
#if defined(FR_ENTRYLO0)
	MFCOP	k1,C0_ENTRYLO_0
	STORE	k1,FR_ENTRYLO0(sp)
#endif
#if defined(FR_ENTRYLO)
	.set push
	.set mips3
	mfc0	k1,C0_ENTRYLO
	STORE	k1,FR_ENTRYLO(sp)
	.set pop
#endif
#if defined(FR_ENTRYLO1)
	.set push
	.set mips3
	MFCOP	k1,C0_ENTRYLO_1
	STORE	k1,FR_ENTRYLO1(sp)
	.set pop
#endif
#if defined(FR_ENTRYHI)
	MFCOP	k1,C0_ENTRYHI
	STORE	k1,FR_ENTRYHI(sp)
#endif
#if defined(FR_PRID)
	mfc0	k1,C0_PRID
	MASK_UW
	STORE	k1,FR_PRID(sp)
#endif
#if defined(FR_LLADDR)
	mfc0	k1,C0_LLADDR
	MASK_UW
	STORE	k1,FR_LLADDR(sp)
#endif
#if defined(FR_CONFIG)
	mfc0	k1,C0_CONFIG
	MASK_UW
	STORE	k1,FR_CONFIG(sp)
#endif
#if defined(FR_DESAVE)
	MFCOP	k1,C0_DESAVE
	STORE	k1,FR_DESAVE(sp)
#endif
#if defined(FR_CACHE)
	mfc0	k1,C0_CACHE
	MASK_UW
	STORE	k1,FR_CACHE(sp)
#endif
#if defined(FR_DEBUG)
	mfc0	k1,C0_DEBUG
	MASK_UW
	STORE	k1,FR_DEBUG(sp)
#endif
#if defined(FR_DEPC)
	MFCOP	k1,C0_DEPC
	STORE	k1,FR_DEPC(sp)
#endif
#if defined(FR_COUNT)
	mfc0	k1,C0_COUNT
	MASK_UW
	STORE	k1,FR_COUNT(sp)
#endif
#if defined(FR_COMPARE)
	mfc0	k1,C0_COMPARE
	MASK_UW
	STORE	k1,FR_COMPARE(sp)
#endif
#if defined(FR_TAGLO)
	mfc0	k1,C0_TAGLO
	MASK_UW
	STORE	k1,FR_TAGLO(sp)
#endif
#if defined(FR_TAGHI)
	mfc0	k1,C0_TAGHI
	MASK_UW
	STORE	k1,FR_TAGHI(sp)
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

#ifndef __R3000
	lui	k1,0
#endif
	mfc0	k1,C0_CAUSE
	move	a1,sp		/* saved regs pointer */
	STORE	k1,FR_CAUSE(sp)

	li	k1,BSP_EXC_SYSCALL
	bne	k1,k0,9f
	 nop
	/* syscall, test for SYS_exit or SYS_interrupt */
	li	k1,SYS_interrupt
	beq	k1,a0,1f
	 li	k0,BSP_EXC_INT
	li	k1,SYS_exit
	bne	k1,a0,9f
	 nop
    1:
	/* adjust pc out of _bsp_trap to facilitate meaningful backtrace */
	/* epc = ra */
	STORE	$31,FR_PC(sp)
	STORE	$31,FR_EPC(sp)
    9:
	/*
	 * enter the debugger stub
	 */
	la	gp,SYM_NAME(_gp)
	la	t0,SYM_NAME(_bsp_dbg_vector)
	LOAD	t0,0(t0)
	move	a1,sp
	jalr	t0
	 move	a0,k0

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
	LOAD	k0,FR_FSR(sp)
	 nop
	mtc1	k0,C1_STATUS
    3:
#endif
	.set reorder
#if defined(FR_INDEX)
	LOAD	k1,FR_INDEX(sp)
	mtc0	k1,C0_INDEX
#endif
#if defined(FR_CONTEXT)
	LOAD	k1,FR_CONTEXT(sp)
	MTCOP	k1,C0_CONTEXT
#endif
#if defined(FR_XCONTEXT)
	LOAD	k1,FR_XCONTEXT(sp)
	MTCOP	k1,C0_XCONTEXT
#endif
#if defined(FR_PAGEMASK)
	LOAD	k1,FR_PAGEMASK(sp)
	mtc0	k1,C0_PAGEMASK
#endif
#if defined(FR_WIRED)
	LOAD	k1,FR_WIRED(sp)
	mtc0	k1,C0_WIRED
#endif
#if defined(FR_ENTRYLO0)
	LOAD	k1,FR_ENTRYLO0(sp)
	MTCOP	k1,C0_ENTRYLO_0
#endif
#if defined(FR_ENTRYLO)
	LOAD	k1,FR_ENTRYLO(sp)
	mtc0	k1,C0_ENTRYLO
#endif
#if defined(FR_ENTRYLO1)
	LOAD	k1,FR_ENTRYLO1(sp)
	MTCOP	k1,C0_ENTRYLO_1
#endif
#if defined(FR_ENTRYHI)
	LOAD	k1,FR_ENTRYHI(sp)
	MTCOP	k1,C0_ENTRYHI
#endif
#if defined(FR_LLADDR)
	LOAD	k1,FR_LLADDR(sp)
	mtc0	k1,C0_LLADDR
#endif
#if defined(FR_DESAVE)
	LOAD	k1,FR_DESAVE(sp)
	MTCOP	k1,C0_DESAVE
#endif
#if defined(FR_DEBUG)
	LOAD	k1,FR_DEBUG(sp)
	mtc0	k1,C0_DEBUG
#endif
#if defined(FR_DEPC)
	LOAD	k1,FR_DEPC(sp)
	MTCOP	k1,C0_DEPC
#endif
#if 0
#if defined(FR_COUNT)
	LOAD	k1,FR_COUNT(sp)
	mtc0	k1,C0_COUNT
#endif
#endif
#if defined(FR_COMPARE)
	LOAD	k1,FR_COMPARE(sp)
	mtc0	k1,C0_COMPARE
#endif

#if defined(FR_TAGLO)
	LOAD	k1,FR_TAGLO(sp)
	mtc0	k1,C0_TAGLO
#endif
#if defined(FR_TAGHI)
	LOAD	k1,FR_TAGHI(sp)
	mtc0	k1,C0_TAGHI
#endif
	.set noreorder

	LOAD	k1,FR_HI(sp)
	LOAD	k0,FR_LO(sp)
	mthi	k1
	LOAD	k1,FR_SR(sp)
	mtlo	k0
	mtc0	k1,C0_STATUS
	LOAD	$31,FR_REG31(sp)
#ifdef __R3000
	LOAD	k0,FR_PC(sp)
#else
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
#if defined(FR_CONFIG)
	LOAD	k1,FR_CONFIG(sp)
#if defined(FR_CACHE)
	/* do this now if we also have a cache register */
	mtc0	k1,C0_CONFIG
#endif
#endif
#if defined(FR_CACHE)
	LOAD	k1,FR_CACHE(sp)
#endif
	LOAD	sp,FR_REG29(sp)

	/* save one of these til last in case cache state changes */
#if defined(FR_CONFIG)
	mtc0	k1,C0_CONFIG
	nop
	nop
	nop
	nop
	nop
#endif
#if defined(FR_CACHE)
	mtc0	k1,C0_CACHE
	nop
	nop
	nop
	nop
	nop
#endif

#ifdef __R3000
	jr	k0
	 rfe
#else
	eret
	 nop
#endif

FUNC_END _bsp_stub_entry
