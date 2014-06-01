/*
 * cpu.h -- CPU specific header for Cygnus BSP.
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
#if !defined(__CPU_H__)
#define __CPU_H__ 1

#if defined(__SIM__)
#if !defined(__DUMMY_SIM_UART__)
#define __SIM_LOADS_DATA__ 1
#endif
#define __CPU_R3900__ 1
#define __CPU_R3904__ 1
#endif

/*
 * __NEED_UNDERSCORE__ is not defined for any MIPS targets
 */
#ifdef __NEED_UNDERSCORE__
#   error __NEED_UNDERSCORE__ is defined and should not be.
#endif

/*
 * Macros to glue together two tokens.
 */
#  ifdef __STDC__
#    define XGLUE(a,b) a##b
#  else
#    define XGLUE(a,b) a/**/b
#  endif

#  define GLUE(a,b) XGLUE(a,b)

/*
 * Symbol Names with leading underscore if necessary
 */
#  ifdef __NEED_UNDERSCORE__
#    define SYM_NAME(name) GLUE(_,name)
#  else
#    define SYM_NAME(name) name
#  endif /* __NEED_UNDERSCORE__ */

/*
 * Various macros to better handle assembler/object format differences
 */
#if defined(__ASSEMBLER__)

/*
 * Assembly function start definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_START name
	.type _\name,@function
	.globl _\name
	.ent   _\name
    _\name:
.endm
#else
.macro FUNC_START name
	.globl \name
	.ent   \name
    \name:
.endm
#endif

/*
 * Assembly function end definition
 */
#ifdef __NEED_UNDERSCORE__
.macro FUNC_END name
    .end _\name
.endm
#else
.macro FUNC_END name
    .end \name
.endm
#endif

#endif /* defined(__ASSEMBLER__) */


#ifdef __ASSEMBLER__
/* Easy to read register names. */
/* Standard MIPS register names: */
#define zero	$0
/*#define at	$1*/
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26	/* kernel private register 0 */
#define k1	$27	/* kernel private register 1 */
#define gp	$28	/* global data pointer */
#define sp 	$29	/* stack-pointer */
#define fp	$30	/* frame-pointer */
#define ra	$31	/* return address */

#define pc	$pc	/* pc, used on mips16 */

#define fp0	$f0
#define fp1	$f1
#endif /* __ASSEMBLER__ */



/* Useful memory constants: */
/*
 * Memory segments (32bit kernel mode addresses)
 */
#define KUSEG                   0x00000000
#define KSEG0                   0x80000000
#define KSEG1                   0xa0000000
#define KSEG2                   0xc0000000
#define KSEG3                   0xe0000000

/*
 * Returns the kernel segment base of a given address
 */
#define KSEGX(a)                (((unsigned long)(a)) & 0xe0000000)

/*
 * Returns the physical address of a KSEG0/KSEG1 address
 */
#define PHYSADDR(a)		(((unsigned long)(a)) & 0x1fffffff)

/*
 * Map an address to a certain kernel segment
 */
#define KSEG0ADDR(a)	((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG0))
#define KSEG1ADDR(a)	((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG1))
#define KSEG2ADDR(a)	((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG2))
#define KSEG3ADDR(a)	((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | KSEG3))

/*
 * Memory segments (64bit kernel mode addresses)
 */
#define XKUSEG     0x0000000000000000
#define XKSSEG     0x4000000000000000
#define XKPHYS     0x8000000000000000
#define XKSEG      0xc000000000000000
#define CKSEG0     0xffffffff80000000
#define CKSEG1     0xffffffffa0000000
#define CKSSEG     0xffffffffc0000000
#define CKSEG3     0xffffffffe0000000

/* Standard Co-Processor 0 register numbers: */
#define C0_INDEX	$0
#define C0_RANDOM	$1
#define C0_ENTRYLO_0	$2
#if defined(__CPU_R3900__)
#define C0_CONFIG	$3
#else
#define C0_ENTRYLO_1	$3
#endif
#define C0_CONTEXT	$4
#define C0_PAGEMASK	$5
#define C0_WIRED	$6
#if defined(__CPU_R3900__)
#define C0_CACHE	$7
#endif
#define C0_BADVA	$8
#define C0_COUNT	$9		/* Count Register */
#define C0_ENTRYHI	$10
#define C0_COMPARE	$11
#define C0_STATUS	$12		/* Status Register */
#define C0_CAUSE	$13		/* last exception description */
#define C0_EPC		$14		/* Exception error address */
#define C0_PRID		$15		/* Processor ID */
#if defined(__CPU_R3900__)
#define C0_DEBUG        $16
#define C0_DEPC         $17
#else
#define C0_CONFIG	$16		/* CPU configuration */
#define C0_LLADDR	$17		/* Load Linked Address */
#endif
#define C0_WATCHLO	$18		/* Watch address low bits */
#define C0_WATCHHI	$19		/* Watch address high bits */
#define C0_XCONTEXT	$20		/* 64-bit addressing context */
#define C0_ECC		$26		/* L2 cache ECC, L1 parity */
#define C0_CACHE_ERR	$27		/* Cache error and status */
#define C0_TAGLO	$28		/* Cache tag register low bits */
#define C0_TAGHI	$29		/* Cache tag register high bits */
#define C0_ERRORPC	$30		/* Error EPC */

#define C1_REVISION     $0
#define C1_STATUS       $31


/* Standard Status Register bitmasks: */
#define SR_CU0		0x10000000	/* Mark CP0 as usable */
#define SR_CU1		0x20000000	/* Mark CP1 as usable */
#define SR_FR		0x04000000	/* Enable MIPS III FP registers */
#define SR_BEV		0x00400000	/* Controls location of exception vectors */
#define SR_DE		0x00010000	/* Cache parity/ecc error control */
#define SR_PE		0x00100000	/* Mark soft reset (clear parity error) */

#define SR_KX		0x00000080	/* Kernel extended addressing enabled */
#define SR_SX		0x00000040	/* Supervisor extended addressing enabled */
#define SR_UX		0x00000020	/* User extended addressing enabled */

#define SR_ERL		0x00000004      /* not on r3000 */

#define SR_TS		0x00200000	/* TLB Shutdown */

/* Standard (R4000) cache operations. Taken from "MIPS R4000
   Microprocessor User's Manual" 2nd edition: */

#define CACHE_I		(0)	/* primary instruction */
#define CACHE_D		(1)	/* primary data */
#define CACHE_SI	(2)	/* secondary instruction */
#define CACHE_SD	(3)	/* secondary data (or combined instruction/data) */

#define INDEX_INVALIDATE		(0)	/* also encodes WRITEBACK if CACHE_D or CACHE_SD */
#define INDEX_LOAD_TAG			(1)
#define INDEX_STORE_TAG			(2)
#define CREATE_DIRTY_EXCLUSIVE		(3)	/* CACHE_D and CACHE_SD only */
#define HIT_INVALIDATE			(4)
#define CACHE_FILL			(5)	/* CACHE_I only */
#define HIT_WRITEBACK_INVALIDATE	(5)	/* CACHE_D and CACHE_SD only */
#define HIT_WRITEBACK			(6)	/* CACHE_I, CACHE_D and CACHE_SD only */
#define HIT_SET_VIRTUAL			(7)	/* CACHE_SI and CACHE_SD only */

#define BUILD_CACHE_OP(o,c)		(((o) << 2) | (c))

/* Individual cache operations: */
#define INDEX_INVALIDATE_I		BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_I)
#define INDEX_WRITEBACK_INVALIDATE_D	BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_D)
#define INDEX_INVALIDATE_SI             BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_SI)
#define INDEX_WRITEBACK_INVALIDATE_SD	BUILD_CACHE_OP(INDEX_INVALIDATE,CACHE_SD)

#define INDEX_LOAD_TAG_I		BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_I)
#define INDEX_LOAD_TAG_D                BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_D)
#define INDEX_LOAD_TAG_SI               BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_SI)
#define INDEX_LOAD_TAG_SD               BUILD_CACHE_OP(INDEX_LOAD_TAG,CACHE_SD)

#define INDEX_STORE_TAG_I              	BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_I)
#define INDEX_STORE_TAG_D               BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_D)
#define INDEX_STORE_TAG_SI              BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_SI)
#define INDEX_STORE_TAG_SD              BUILD_CACHE_OP(INDEX_STORE_TAG,CACHE_SD)

#define CREATE_DIRTY_EXCLUSIVE_D        BUILD_CACHE_OP(CREATE_DIRTY_EXCLUSIVE,CACHE_D)
#define CREATE_DIRTY_EXCLUSIVE_SD       BUILD_CACHE_OP(CREATE_DIRTY_EXCLUSIVE,CACHE_SD)

#define HIT_INVALIDATE_I                BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_I)
#define HIT_INVALIDATE_D                BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_D)
#define HIT_INVALIDATE_SI               BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_SI)
#define HIT_INVALIDATE_SD               BUILD_CACHE_OP(HIT_INVALIDATE,CACHE_SD)

#define CACHE_FILL_I                    BUILD_CACHE_OP(CACHE_FILL,CACHE_I)
#define HIT_WRITEBACK_INVALIDATE_D      BUILD_CACHE_OP(HIT_WRITEBACK_INVALIDATE,CACHE_D)
#define HIT_WRITEBACK_INVALIDATE_SD     BUILD_CACHE_OP(HIT_WRITEBACK_INVALIDATE,CACHE_SD)

#define HIT_WRITEBACK_I                 BUILD_CACHE_OP(HIT_WRITEBACK,CACHE_I)
#define HIT_WRITEBACK_D                 BUILD_CACHE_OP(HIT_WRITEBACK,CACHE_D)
#define HIT_WRITEBACK_SD                BUILD_CACHE_OP(HIT_WRITEBACK,CACHE_SD)

#define HIT_SET_VIRTUAL_SI		BUILD_CACHE_OP(HIT_SET_VIRTUAL,CACHE_SI)
#define HIT_SET_VIRTUAL_SD              BUILD_CACHE_OP(HIT_SET_VIRTUAL,CACHE_SD)


#ifdef __mips_soft_float
#define CU1_FLAG 0
#else
#define CU1_FLAG SR_CU1
#endif

#if defined(__CPU_R3900__)
#define INIT_SR	(SR_BEV | 0xff00)
#define NMI_SHARES_RESET 1
#define HAVE_TX_REGS 1
#if !defined(CHECK_FOR_NMI)
#if defined(__SIM__)
#define	CHECK_FOR_NMI() \
	move	k0,zero
#else
#define	CHECK_FOR_NMI() \
	mfc0	k0,C0_STATUS ; \
	srl	k0,k0,20 ; \
	andi	k0,k0,1
#endif
#endif /* CHECK_FOR_NMI */
#endif /* __CPU_R3900__ */



/*
 * Some R3904 Internal registers.
 */
#if defined(__CPU_R3904__)
#define R3904_ISR     (*(unsigned int *)0xffffc000)
#define R3904_IMR     (*(unsigned int *)0xffffc004)
#define R3904_ILRBASE ((unsigned int *)0xffffc010)

#define R3904_COCR    (*(unsigned int *)0xffffe000)
#define R3904_IACK    (*(unsigned char *)0xffffe001)
#define R3904_ICFG    (*(unsigned short *)0xffffe002)
#endif

#if !defined(__ASSEMBLER__)

#define BREAKPOINT() asm volatile(" .long 0x0005000d")
#define SYSCALL      syscall

#define BSP_ENABLE_INTERRUPTS()    asm volatile("
	.set noreorder
	.set noat
	mfc0 $1,$12
	nop
	ori  $1,$1,1
	mtc0 $1,$12
	nop
	nop
	nop
	.set at
	.set reorder")
#endif /* defined(__ASSEMBLER__) */

/*
 *  breakpoint opcode.
 */
#define BREAKPOINT_OPCODE	0x0005000d

/*
 * Core Exception vectors.
 */
#define BSP_EXC_INT	    0
#define BSP_EXC_TLBMOD	    1
#define BSP_EXC_TLBL	    2
#define BSP_EXC_TLBS	    3
#define BSP_EXC_ADEL	    4
#define BSP_EXC_ADES	    5
#define BSP_EXC_IBE         6
#define BSP_EXC_DBE         7
#define BSP_EXC_SYSCALL     8
#define BSP_EXC_BREAK       9
#define BSP_EXC_ILL        10
#define BSP_EXC_CPU        11
#define BSP_EXC_OV         12
#define BSP_EXC_TRAP       13
#define BSP_EXC_VCEI       14
#define BSP_EXC_FPE        15
#define BSP_EXC_RSV16      16
#define BSP_EXC_RSV17      17
#define BSP_EXC_RSV18      18
#define BSP_EXC_RSV19      19
#define BSP_EXC_RSV20      20
#define BSP_EXC_RSV21      21
#define BSP_EXC_RSV22      22
#define BSP_EXC_WATCH      23
#define BSP_EXC_RSV24      24
#define BSP_EXC_RSV25      25
#define BSP_EXC_RSV26      26
#define BSP_EXC_RSV27      27
#define BSP_EXC_RSV28      28
#define BSP_EXC_RSV29      29
#define BSP_EXC_RSV30      30
#define BSP_EXC_VCED       31
/* tx39 debug exception */
#define BSP_EXC_DEBUG      32
#define BSP_EXC_TLB        33
#define BSP_EXC_NMI        34
/*
 * Hack for eCos on tx39 to set an async breakpoint.
 */
#define BSP_VEC_BP_HOOK    35

#define BSP_EXC_XTLB	   36
#define BSP_EXC_CACHE	   37

#define BSP_MAX_EXCEPTIONS 38

/*
 * Another hack for tx39 eCos compatibility.
 */
#if defined(__CPU_R3900__)
#define BSP_VEC_MT_DEBUG   15
#else
#define BSP_VEC_MT_DEBUG   38
#endif

#define BSP_VEC_STUB_ENTRY 39
#define BSP_VEC_BSPDATA    40
#define BSP_VEC_MAGIC      41
#define BSP_VEC_IRQ_CHECK  42

#define BSP_VEC_PAD        43
#define NUM_VTAB_ENTRIES   44

/*
 *  Interrupts.
 */
#define BSP_IRQ_SW0	0
#define BSP_IRQ_SW1	1
#define BSP_IRQ_HW0	2
#define BSP_IRQ_HW1	3
#define BSP_IRQ_HW2	4
#define BSP_IRQ_HW3	5
#define BSP_IRQ_HW4	6
#define BSP_IRQ_HW5	7

#if defined(__CPU_R3904__)
#define BSP_IRQ_HW6	8
#define BSP_IRQ_HW7	9
#define BSP_IRQ_DMA3   10
#define BSP_IRQ_DMA2   11
#define BSP_IRQ_DMA1   12
#define BSP_IRQ_DMA0   13
#define BSP_IRQ_SIO0   14
#define BSP_IRQ_SIO1   15
#define BSP_IRQ_TMR0   16
#define BSP_IRQ_TMR1   17
#define BSP_IRQ_TMR2   18
#endif


#ifdef __R3000
#define GPR_SIZE 4
#define FPR_SIZE 4
#else
#define GPR_SIZE 8
#define FPR_SIZE 8
#endif

#define FR_REG0	  0
#define FR_REG1	  (FR_REG0 + GPR_SIZE)
#define FR_REG2	  (FR_REG1 + GPR_SIZE)
#define FR_REG3	  (FR_REG2 + GPR_SIZE)
#define FR_REG4	  (FR_REG3 + GPR_SIZE)
#define FR_REG5	  (FR_REG4 + GPR_SIZE)
#define FR_REG6	  (FR_REG5 + GPR_SIZE)
#define FR_REG7	  (FR_REG6 + GPR_SIZE)
#define FR_REG8	  (FR_REG7 + GPR_SIZE)
#define FR_REG9	  (FR_REG8 + GPR_SIZE)
#define FR_REG10  (FR_REG9 + GPR_SIZE)
#define FR_REG11  (FR_REG10 + GPR_SIZE)
#define FR_REG12  (FR_REG11 + GPR_SIZE)
#define FR_REG13  (FR_REG12 + GPR_SIZE)
#define FR_REG14  (FR_REG13 + GPR_SIZE)
#define FR_REG15  (FR_REG14 + GPR_SIZE)
#define FR_REG16  (FR_REG15 + GPR_SIZE)
#define FR_REG17  (FR_REG16 + GPR_SIZE)
#define FR_REG18  (FR_REG17 + GPR_SIZE)
#define FR_REG19  (FR_REG18 + GPR_SIZE)
#define FR_REG20  (FR_REG19 + GPR_SIZE)
#define FR_REG21  (FR_REG20 + GPR_SIZE)
#define FR_REG22  (FR_REG21 + GPR_SIZE)
#define FR_REG23  (FR_REG22 + GPR_SIZE)
#define FR_REG24  (FR_REG23 + GPR_SIZE)
#define FR_REG25  (FR_REG24 + GPR_SIZE)
#define FR_REG26  (FR_REG25 + GPR_SIZE)
#define FR_REG27  (FR_REG26 + GPR_SIZE)
#define FR_REG28  (FR_REG27 + GPR_SIZE)
#define FR_REG29  (FR_REG28 + GPR_SIZE)
#define FR_REG30  (FR_REG29 + GPR_SIZE)
#define FR_REG31  (FR_REG30 + GPR_SIZE)

#define FR_SR	  (FR_REG31 + GPR_SIZE)
#define FR_LO     (FR_SR    + GPR_SIZE)
#define FR_HI	  (FR_LO    + GPR_SIZE)
#define FR_BAD	  (FR_HI    + GPR_SIZE)
#define FR_CAUSE  (FR_BAD   + GPR_SIZE)
#define FR_PC	  (FR_CAUSE + GPR_SIZE)

#define FR_FREG0  (FR_PC    + GPR_SIZE)
#define FR_FREG1  (FR_FREG0 + FPR_SIZE)
#define FR_FREG2  (FR_FREG1 + FPR_SIZE)
#define FR_FREG3  (FR_FREG2 + FPR_SIZE)
#define FR_FREG4  (FR_FREG3 + FPR_SIZE)
#define FR_FREG5  (FR_FREG4 + FPR_SIZE)
#define FR_FREG6  (FR_FREG5 + FPR_SIZE)
#define FR_FREG7  (FR_FREG6 + FPR_SIZE)
#define FR_FREG8  (FR_FREG7 + FPR_SIZE)
#define FR_FREG9  (FR_FREG8 + FPR_SIZE)
#define FR_FREG10 (FR_FREG9 + FPR_SIZE)
#define FR_FREG11 (FR_FREG10 + FPR_SIZE)
#define FR_FREG12 (FR_FREG11 + FPR_SIZE)
#define FR_FREG13 (FR_FREG12 + FPR_SIZE)
#define FR_FREG14 (FR_FREG13 + FPR_SIZE)
#define FR_FREG15 (FR_FREG14 + FPR_SIZE)
#define FR_FREG16 (FR_FREG15 + FPR_SIZE)
#define FR_FREG17 (FR_FREG16 + FPR_SIZE)
#define FR_FREG18 (FR_FREG17 + FPR_SIZE)
#define FR_FREG19 (FR_FREG18 + FPR_SIZE)
#define FR_FREG20 (FR_FREG19 + FPR_SIZE)
#define FR_FREG21 (FR_FREG20 + FPR_SIZE)
#define FR_FREG22 (FR_FREG21 + FPR_SIZE)
#define FR_FREG23 (FR_FREG22 + FPR_SIZE)
#define FR_FREG24 (FR_FREG23 + FPR_SIZE)
#define FR_FREG25 (FR_FREG24 + FPR_SIZE)
#define FR_FREG26 (FR_FREG25 + FPR_SIZE)
#define FR_FREG27 (FR_FREG26 + FPR_SIZE)
#define FR_FREG28 (FR_FREG27 + FPR_SIZE)
#define FR_FREG29 (FR_FREG28 + FPR_SIZE)
#define FR_FREG30 (FR_FREG29 + FPR_SIZE)
#define FR_FREG31 (FR_FREG30 + FPR_SIZE)

#define FR_FSR    (FR_FREG31 + FPR_SIZE)
#define FR_FIR    (FR_FSR    + GPR_SIZE)
#define FR_XFP    (FR_FIR    + GPR_SIZE)

#define FR_REG73  (FR_XFP    + GPR_SIZE)
#define FR_REG74  (FR_REG73  + GPR_SIZE)
#define FR_REG75  (FR_REG74  + GPR_SIZE)
#define FR_REG76  (FR_REG75  + GPR_SIZE)
#define FR_REG77  (FR_REG76  + GPR_SIZE)
#define FR_REG78  (FR_REG77  + GPR_SIZE)
#define FR_REG79  (FR_REG78  + GPR_SIZE)
#define FR_REG80  (FR_REG79  + GPR_SIZE)
#define FR_REG81  (FR_REG80  + GPR_SIZE)
#define FR_REG82  (FR_REG81  + GPR_SIZE)
#define FR_REG83  (FR_REG82  + GPR_SIZE)
#define FR_REG84  (FR_REG83  + GPR_SIZE)
#define FR_REG85  (FR_REG84  + GPR_SIZE)
#define FR_REG86  (FR_REG85  + GPR_SIZE)
#define FR_REG87  (FR_REG86  + GPR_SIZE)
#define FR_REG88  (FR_REG87  + GPR_SIZE)
#define FR_REG89  (FR_REG88  + GPR_SIZE)
#define FR_REG90  (FR_REG89  + GPR_SIZE)
#define FR_REG91  (FR_REG90  + GPR_SIZE)
#define FR_REG92  (FR_REG91  + GPR_SIZE)
#define FR_EPC    (FR_REG92  + GPR_SIZE)
#define FR_REG94  (FR_EPC    + GPR_SIZE)
#define FR_EXC_NR (FR_REG94  + GPR_SIZE)

#define EX_STACK_SIZE (FR_EXC_NR + GPR_SIZE)

#if NMI_SHARES_RESET
#define SR_NMI          0x00100000      /* Non-Maskable Interrupt status bit */
#endif


/*
 * Here is where we map implementation specific regs
 * into the general purpose stackframe slots.
 */
#if defined(HAVE_TX_REGS)
#ifdef __R3000
#define REG_CONFIG REG_EMB84
#define FR_CONFIG  FR_REG84

#define REG_CACHE  REG_EMB85
#define FR_CACHE   FR_REG85

#define REG_DEBUG  REG_EMB86
#define FR_DEBUG   FR_REG86

#define REG_DEPC   REG_EMB87
#define FR_DEPC    FR_REG87

#define REG_EEPC   REG_EMB88
#define FR_EEPC    FR_REG88
#else
#define REG_DESAVE REG_EMB79
#define FR_DESAVE  FR_REG79

#define REG_CONFIG REG_EMB84
#define FR_CONFIG  FR_REG84

#define REG_DEBUG  REG_EMB87
#define FR_DEBUG   FR_REG87

#define REG_DEPC   REG_EMB88
#define FR_DEPC    FR_REG88

#define REG_EEPC   REG_EMB91
#define FR_EEPC    FR_REG91
#endif
#endif  /* HAVE_TX_REGS */

#if !defined(REG_CONFIG)
#define REG_CONFIG REG_EMB84
#define FR_CONFIG  FR_REG84
#endif
#if !defined(REG_EEPC)
#define REG_EEPC   REG_EMB91
#define FR_EEPC    FR_REG91
#endif

#if NUM_TLB_ENTRIES
#define REG_INDEX  REG_EMB73
#define FR_INDEX   FR_REG73

#ifdef __R3000
#define REG_ENTRYLO   REG_EMB74
#define FR_ENTRYLO    FR_REG74
#else
#define REG_ENTRYLO0  REG_EMB74
#define FR_ENTRYLO0   FR_REG74

#define REG_ENTRYLO1  REG_EMB75
#define FR_ENTRYLO1   FR_REG75
#endif

#define REG_CONTEXT  REG_EMB76
#define FR_CONTEXT   FR_REG76

#ifndef __R3000
#define REG_PAGEMASK  REG_EMB77
#define FR_PAGEMASK   FR_REG77

#define REG_WIRED  REG_EMB78
#define FR_WIRED   FR_REG78
#endif

#define REG_ENTRYHI  REG_EMB81
#define FR_ENTRYHI   FR_REG81

#define REG_XCONTEXT  REG_EMB86
#define FR_XCONTEXT   FR_REG86
#endif /* NUM_TLB_ENTRIES */


#define REG_PRID  REG_EMB83
#define FR_PRID   FR_REG83

#ifndef __R3000
#define REG_COUNT   REG_EMB80
#define FR_COUNT    FR_REG80

#define REG_COMPARE   REG_EMB82
#define FR_COMPARE    FR_REG82

#define REG_LLADDR  REG_EMB85
#define FR_LLADDR   FR_REG85

#define REG_TAGLO   REG_EMB89
#define FR_TAGLO    FR_REG89

#define REG_TAGHI   REG_EMB90
#define FR_TAGHI    FR_REG90
#endif  /* !__R3000 */




#if !defined(__ASSEMBLER__)
  /*
   * Register name that is used in help strings and such
   */
# define REGNAME_EXAMPLE "a0"

enum __regnames {
    REG_ZERO,   REG_AT,     REG_V0,     REG_V1,
    REG_A0,     REG_A1,     REG_A2,     REG_A3,
    REG_T0,     REG_T1,     REG_T2,     REG_T3,
    REG_T4,     REG_T5,     REG_T6,     REG_T7,
    REG_S0,     REG_S1,     REG_S2,     REG_S3,
    REG_S4,     REG_S5,     REG_S6,     REG_S7,
    REG_T8,     REG_T9,     REG_K0,     REG_K1,
    REG_GP,     REG_SP,     REG_S8,     REG_RA,
    REG_SR,     REG_LO,     REG_HI,     REG_BAD,
    REG_CAUSE,  REG_PC,
    REG_F0,     REG_F1,     REG_F2,     REG_F3,
    REG_F4,     REG_F5,     REG_F6,     REG_F7,
    REG_F8,     REG_F9,     REG_F10,    REG_F11,
    REG_F12,    REG_F13,    REG_F14,    REG_F15,
    REG_F16,    REG_F17,    REG_F18,    REG_F19,
    REG_F20,    REG_F21,    REG_F22,    REG_F23,
    REG_F24,    REG_F25,    REG_F26,    REG_F27,
    REG_F28,    REG_F29,    REG_F30,    REG_F31,
    REG_FSR,    REG_FIR,    REG_FP,     REG_EMB73,

    REG_EMB74,  REG_EMB75,  REG_EMB76,  REG_EMB77,
    REG_EMB78,  REG_EMB79,  REG_EMB80,  REG_EMB81,
    REG_EMB82,  REG_EMB83,  REG_EMB84,  REG_EMB85, 
    REG_EMB86,  REG_EMB87,  REG_EMB88,  REG_EMB89,

    /* gdb doesn't know about these (yet) */
    REG_EMB90,  REG_EMB91,  REG_EMB92,  REG_EPC,
    REG_EMB94,  REG_EXC_NR,

    REG_LAST
};

/*
 *  How registers are stored for exceptions.
 */
typedef struct
{
    unsigned long _zero;
    unsigned long _at;
    unsigned long _v0;
    unsigned long _v1;
    unsigned long _a0;
    unsigned long _a1;
    unsigned long _a2;
    unsigned long _a3;
    unsigned long _t0;
    unsigned long _t1;
    unsigned long _t2;
    unsigned long _t3;
    unsigned long _t4;
    unsigned long _t5;
    unsigned long _t6;
    unsigned long _t7;
    unsigned long _s0;
    unsigned long _s1;
    unsigned long _s2;
    unsigned long _s3;
    unsigned long _s4;
    unsigned long _s5;
    unsigned long _s6;
    unsigned long _s7;
    unsigned long _t8;
    unsigned long _t9;
    unsigned long _k0;
    unsigned long _k1;
    unsigned long _gp;
    unsigned long _sp;
    unsigned long _fp;
    unsigned long _ra;

    unsigned long _sr;
    unsigned long _lo;
    unsigned long _hi;
    unsigned long _bad;
    unsigned long _cause;
    unsigned long _pc;

#ifdef __R3000
    float	  _fpr[32];
#else
    double	  _fpr[32];
#endif

    unsigned long _fsr;
    unsigned long _fir;
    unsigned long _dummyfp;
    unsigned long _emb73;

    unsigned long _emb74;
    unsigned long _emb75;
    unsigned long _emb76;
    unsigned long _emb77;
    unsigned long _emb78;
    unsigned long _emb79;
    unsigned long _emb80;
    unsigned long _emb81;
    unsigned long _emb82;
    unsigned long _emb83;
    unsigned long _emb84;
    unsigned long _emb85;
    unsigned long _emb86;
    unsigned long _emb87;
    unsigned long _emb88;
    unsigned long _emb89;
    unsigned long _emb90;
#ifdef __R3000
    unsigned long _emb91;
#else
    unsigned long _eepc;
#endif
    unsigned long _emb92;
    unsigned long _epc;
    unsigned long _emb94;
    unsigned long _exc_nr;
} ex_regs_t;

typedef ex_regs_t gdb_regs_t;



#if NUM_TLB_ENTRIES
extern void __tlb_init(void);
#endif

extern void __dcache_flush(void *addr, int nbytes);
extern void __icache_flush(void *addr, int nbytes);
extern int  __dcache_disable(void);
extern void __dcache_enable(void);
extern void __icache_disable(void);
extern void __icache_enable(void);

static inline void __set_sr(unsigned val)
{
    asm volatile (
        "mtc0   %0,$12\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_sr(void)
{
    unsigned retval;

    asm volatile (
        "mfc0   %0,$12\nnop\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline unsigned __get_badvaddr(void)
{
    unsigned retval;

    asm volatile (
        "mfc0   %0,$8\nnop\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

static inline unsigned __get_epc(void)
{
    unsigned retval;

    asm volatile (
        "mfc0   %0,$14\nnop\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

#define IRQ_FLAG unsigned
#define DISABLE_IRQ(x) { x = __get_sr(); __set_ps(x & ~1); }
#define RESTORE_IRQ(x) __set_ps(x)

#if defined(__CPU_R3900__)
#define __wbflush()   \
  asm volatile(       \
    "nop\n"           \
    "sync\n"          \
    "nop\n"           \
    "1: bc0f 1b\n"    \
    "nop\n"           \
  )
#else
#define __wbflush()
#endif

/*
 * these are hooks for board-specific code to validate read/write
 * access for boards which don't have HW support for bus-errors
 * in certain memory regions. They return true if the memory
 * range is valid, false if not.
 */
extern int (*__mem_read_validate_hook)(void *addr, int len);
extern int (*__mem_write_validate_hook)(void *addr, int len);

#endif /* !__ASSEMBLER__ */

#endif /* !defined(__CPU_H__) */
