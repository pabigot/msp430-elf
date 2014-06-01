/*
 * vectors.S -- Exception vector code for ARM(R) BSP
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include <bsp/cpu.h>

	.file	"vectors.S"

	.text
	

/*
 *  Create exception frame and switch to SVC mode from a non-SVC mode
 *  exception .
 */
.macro SWITCH_TO_SVC_FROM_NONSVC
	/* save some working regs on mode-specific stack */
	stmfd	sp!, {r0-r1}
	mov	r0, lr	     /* pre-exception pc     */
	mrs	r1, spsr     /* pre-exception  cpsr  */
	stmfd	sp!, {r0-r1}
	mov	r0, sp	     /* pointer to stuff we just saved */

	/* switch to SVC mode */
	mrs	r1, cpsr
	and	r1, r1, IMM(~ARM_PSR_MODE_MASK)
	orr	r1, r1, IMM(ARM_PSR_MODE_SVC)
	msr	cpsr, r1
	stmfd	r0!, {sp}    /* save sp in temp area */

	/***************************************
	 * r0 points to our temp save area:    *
	 *				       *
	 *  +16:  pre-exception r1	       *
	 *  +12:  pre-exception r0	       *
	 *   +8:  pre-exception cpsr	       *
	 *   +4:  pre-exception pc	       *
	 *   +0:  saved sp_svc		       *
	 **************************************/

	/* switch to exception stack */
	ldr	r1, =SYM_NAME(_ex_stack)
	cmp	sp, r1
	subhi	sp, r1, IMM(ARM_EX_REGS_T_SIZE)
	subls	sp, sp, IMM(ARM_EX_REGS_T_SIZE)
	
	stmia	sp, {r0-r7}

	/* fixup regs from temp save area */
	ldmfd	r0, {r2-r6}
	str	r2, [sp, IMM(spsvc_o)]
	str	r3, [sp, IMM(pc_o)]
	str	r4, [sp, IMM(cpsr_o)]
	str	r5, [sp, IMM(r0_o)]
	str	r6, [sp, IMM(r1_o)]

	/* switch to pre-exception mode to get banked regs */
	add	r0, sp, IMM(r8_o)
	mrs	r1, cpsr
	orr	r4, r4, IMM(ARM_PSR_IRQ|ARM_PSR_FIQ)
	msr	cpsr, r4
	stmia	r0, {r8-r12,sp,lr}
	/* back to svc mode */
	msr	cpsr, r1

	/* if pre-exception mode is svc mode, fixup sp in exception frame */
	eor	r1, r1, r4
	ands	r1, r1, IMM(ARM_PSR_MODE_MASK)
	ldreq	r0, [sp, IMM(spsvc_o)]
	streq	r0, [sp, IMM(sp_o)]
.endm


/*
 *  Create exception frame from a SVC mode exception.
 */
.macro SWITCH_TO_SVC_FROM_SVC
	/* save some working regs on current stack */
	str	r0, [sp, IMM(-12)]
	str	r1, [sp, IMM(-8)]
	str	sp, [sp, IMM(-4)]
	sub	sp, sp, IMM(12)
	/*
	 *  osp:
	 *  osp-4:   osp
	 *  osp-8:   r1
	 *  osp-12:  r0
	 *
	 *  sp = osp-12
	 */
	mov	r0, sp

	/* switch to exception stack and create frame */
	ldr	r1, =SYM_NAME(_ex_stack)
	cmp	sp, r1
	subhi	sp, r1, IMM(ARM_EX_REGS_T_SIZE)
	subls	sp, sp, IMM(ARM_EX_REGS_T_SIZE - 8)

	/* sp now points to base of exception frame area */
	
	/*
	 * Fill in the exception frame
	 */
	ldr	r1, [r0, IMM(8)]	/* get osp */
	str	r1, [sp, IMM(spsvc_o)]	/* save it in exception frame */
	ldmia	r0, {r0-r1}	
	stmia	sp, {r0-r7}
	mrs	r4, spsr     
	str	r4, [sp, IMM(cpsr_o)] /* pre-exception	cpsr	  */
	str	lr, [sp, IMM(pc_o)]   /* exception return address */

	/* switch to pre-exception mode to get banked regs */
	add	r0, sp, IMM(r8_o)
	mrs	r1, cpsr
	orr	r4, r4, IMM(ARM_PSR_IRQ|ARM_PSR_FIQ)
	msr	cpsr, r4
	stmia	r0, {r8-r12,sp,lr}
	/* back to svc mode */
	msr	cpsr, r1

	/* if pre-exception mode is svc mode, fixup sp in exception frame */
	eor	r1, r1, r4
	ands	r1, r1, IMM(ARM_PSR_MODE_MASK)
	ldreq	r0, [sp, IMM(spsvc_o)]
	streq	r0, [sp, IMM(sp_o)]
.endm


.macro COMMON_SVC
	/*
	 * void generic_exception_handler(ex_regs_t *regs,
	 *				  unsigned long vect_num)
	 */
	mov	r0, sp
	bl	SYM_NAME(generic_exception_handler)

	/* restore pre-exception mode r8-12,sp,lr */
	add	r0, sp, IMM(r8_o)
	mrs	r1, cpsr
	ldr	r4, [sp, IMM(cpsr_o)]
	orr	r3, r4, IMM(ARM_PSR_IRQ|ARM_PSR_FIQ)
	msr	cpsr, r3	/* switch mode */
	ldmia	r0!, {r8-r12}
	ldr	lr, [r0, IMM(4)]

	/* if pre-exception mode is not svc mode, fix sp */
	eor	r3, r3, r1
	ands	r3, r3, IMM(ARM_PSR_MODE_MASK)
	ldrne	sp, [r0]
	msr	cpsr, r1	/* back to svc mode */
	
	ldr	r0, [sp, IMM(spsvc_o)]
	str	r0, [sp, IMM(pc_o) - 4]
	msr	spsr, r4
	ldmia	sp, {r0-r7}
	add	sp, sp, IMM(pc_o) - 4
	ldmia	sp, {sp,pc}^
.endm

/*
 * This is the Undefined instruction interrupt handler
 */
	.p2align 5
FUNC_START undefined_isr_asm
	/*
	 * Set up a temp stack to use
	 */
	ldr	sp, =SYM_NAME(_undefined_stack)

	SWITCH_TO_SVC_FROM_NONSVC
	ldr	r1, =BSP_CORE_EXC_UNDEFINED_INSTRUCTION
	COMMON_SVC
FUNC_END undefined_isr_asm


/*
 * This is the SWI interrupt handler
 */
	.p2align 5
FUNC_START swi_isr_asm
	SWITCH_TO_SVC_FROM_SVC
	ldr	r1, =BSP_CORE_EXC_SOFTWARE_INTERRUPT
	COMMON_SVC
FUNC_END swi_isr_asm


/*
 * This is the Prefetch Abort interrupt handler
 */
	.p2align 5
FUNC_START prefetch_abort_isr_asm
	/*
	 * Set up a temp stack to use
	 */
	ldr	sp, =SYM_NAME(_abort_stack)
	sub	lr, lr, IMM(4)

	SWITCH_TO_SVC_FROM_NONSVC
	ldr	r1, =BSP_CORE_EXC_PREFETCH_ABORT
	COMMON_SVC
FUNC_END prefetch_abort_isr_asm


/*
 * This is the Data Abort interrupt handler
 */
	.p2align 5
FUNC_START data_abort_isr_asm
	/*
	 * Set up a temp stack to use
	 */
	ldr	sp, =SYM_NAME(_abort_stack)
	sub	lr, lr, IMM(8)

	SWITCH_TO_SVC_FROM_NONSVC
	ldr	r1, =BSP_CORE_EXC_DATA_ABORT
	COMMON_SVC
FUNC_END data_abort_isr_asm


/*
 * This is the 26-bit mode Address Error interrupt handler
 */
	.p2align 5
FUNC_START address_err_isr_asm
	/*
	 * Set up a temp stack to use
	 */
	ldr	sp, =SYM_NAME(_address_err_stack)
	sub	lr, lr, IMM(4)

	SWITCH_TO_SVC_FROM_NONSVC
	ldr	r1, =BSP_CORE_EXC_ADDRESS_ERROR_26_BIT
	COMMON_SVC
FUNC_END address_err_isr_asm

/*
 * This is the IRQ interrupt handler
 */
	.p2align 5
FUNC_START irq_isr_asm
	/*
	 * Set up a temp stack to use
	 */
	ldr	sp, =SYM_NAME(_irq_stack)
	sub	lr, lr, IMM(4)

	SWITCH_TO_SVC_FROM_NONSVC
	ldr	r1, =BSP_CORE_EXC_IRQ
	COMMON_SVC
FUNC_END irq_isr_asm

/*
 * This is the FIQ interrupt handler
 */
	.p2align 5
FUNC_START fiq_isr_asm
	/*
	 * Set up a temp stack to use
	 */
	ldr	sp, =SYM_NAME(_fiq_stack)
	sub	lr, lr, IMM(4)

	SWITCH_TO_SVC_FROM_NONSVC
	ldr	r1, =BSP_CORE_EXC_FIQ
	COMMON_SVC
FUNC_END fiq_isr_asm


/*
 * Assembly representing the exception vectors.
 */
	.section .ram_vectors

SYM_NAME(RESET_VECTOR):		  b	  SYM_NAME(RESET_VECTOR)
	.globl SYM_NAME(UNDEFINED_VECTOR)
SYM_NAME(UNDEFINED_VECTOR):	  ldr	  pc, SYM_NAME(ROM_UNDEFINED_ISR)
	.globl SYM_NAME(SWI_VECTOR)
SYM_NAME(SWI_VECTOR):		  ldr	  pc, SYM_NAME(ROM_SWI_ISR)
	.globl SYM_NAME(PREFETCH_ABORT_VECTOR)
SYM_NAME(PREFETCH_ABORT_VECTOR):  ldr	  pc, SYM_NAME(ROM_PREFETCH_ABORT_ISR)
	.globl SYM_NAME(DATA_ABORT_VECTOR)
SYM_NAME(DATA_ABORT_VECTOR):	  ldr	  pc, SYM_NAME(ROM_DATA_ABORT_ISR)
	.globl SYM_NAME(ADDR_ERROR_VECTOR)
SYM_NAME(ADDR_ERROR_VECTOR):	  ldr	  pc, SYM_NAME(ROM_ADDR_ERROR_ISR)
	.globl SYM_NAME(IRQ_VECTOR)
SYM_NAME(IRQ_VECTOR):		  ldr	  pc, SYM_NAME(ROM_IRQ_ISR)
	.globl SYM_NAME(FIQ_VECTOR)
SYM_NAME(FIQ_VECTOR):		  ldr	  pc, SYM_NAME(ROM_FIQ_ISR)

SYM_NAME(ROM_UNDEFINED_ISR):	  .word	  SYM_NAME(undefined_isr_asm)
SYM_NAME(ROM_SWI_ISR):		  .word	  SYM_NAME(swi_isr_asm)
SYM_NAME(ROM_PREFETCH_ABORT_ISR): .word	  SYM_NAME(prefetch_abort_isr_asm)
SYM_NAME(ROM_DATA_ABORT_ISR):	  .word	  SYM_NAME(data_abort_isr_asm)
SYM_NAME(ROM_ADDR_ERROR_ISR):	  .word	  SYM_NAME(address_err_isr_asm)
SYM_NAME(ROM_IRQ_ISR):		  .word	  SYM_NAME(irq_isr_asm)
SYM_NAME(ROM_FIQ_ISR):		  .word	  SYM_NAME(fiq_isr_asm)


#ifdef __YAVAPAI__
	/* we have to skip over Yavapai internal regs */
	.p2align 13
	.global SYM_NAME(__REAL_DATA_START)
SYM_NAME(__REAL_DATA_START):
#endif


#ifdef MMU
/*
 * These are the MMU page tables
 */
	.macro FL_SECTION_ENTRY base,x,ap,p,d,c,b
	.word (\base << 20) | (\x << 12) | (\ap << 10) | (\p << 9) | (\d << 5) | (\c << 3) | (\b << 2) | 2
	.endm

	/*
	 * I wanted to use logical operations here, but since I am using symbols later 
	 * to fill in the parameters, I had to use addition to force the assembler to
	 * do it right
	 */
	.macro FL_PT_ENTRY base,d
	.word \base + (\d << 5) + 1
	.endm

	.macro SL_SMPAGE_ENTRY base,ap3,ap2,ap1,ap0,c,b
	.word (\base << 12) | (\ap3 << 10) | (\ap2 << 8) | (\ap1 << 6) | (\ap0 << 4) | (\c << 3) | (\b << 2) | 2
	.endm

#if defined(__BOARD_XARET__)
#ifdef __ELF__
    /* "a" defines this section to be allocatable.  Without that, the binary objdump will strip this section */
	.section .rom_sec_table, "a"
#else
    /* "a" is an unknown attribute for coff targets. */
	.section .rom_sec_table
#endif
	.global SYM_NAME(page1)
SYM_NAME(page1):
	FL_SECTION_ENTRY 0x000,0,0,0,0,1,1
	FL_SECTION_ENTRY 0x001,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x002,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x003,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x004,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x005,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x006,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x007,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x008,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x009,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x00A,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x00B,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x00C,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x00D,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x00E,0,3,0,0,0,0
	FL_SECTION_ENTRY 0x00F,0,3,0,0,0,0
	.set	__base,0x010
	.rept 0xA00 - 0x010
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr
	.rept 0xA20 - 0xA00
	FL_SECTION_ENTRY __base,1,3,1,0,1,1
	.set	__base,__base+1
	.endr
	.rept 0x1000 - 0xA20
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr
#elif defined(__BOARD_IQ80310__)
#ifdef __ELF__
    /* "a" defines this section to be allocatable.  Without that, the binary objdump will strip this section */
	.section .rom_sec_table, "a"
#else
    /* "a" is an unknown attribute for coff targets. */
	.section .rom_sec_table
#endif
	.global SYM_NAME(page1)

SYM_NAME(page1):
	/*
	 * 8MB of FLASH and i80307 MMRs mapped in using 4K small pages so we can
	 * set the access permission on flash and memory-mapped registers properly
	 */
	.rept	0x8
	FL_PT_ENTRY SYM_NAME(page2),0
	.endr	

	.set	__base,0x8
	.rept	0xA00 - 0x8		/* nothing interesting here */
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

	.rept	0xB00 - 0xA00		/* 32MB ECC SDRAM */
	FL_SECTION_ENTRY __base,1,3,1,0,1,1	/* ECC, x=c=b=1 */
	.set	__base,__base+1
	.endr
	
	.rept	0x1000 - 0xB00		/* only I/O at 0xFE8xxxxx */
	FL_SECTION_ENTRY __base,0,3,0,0,0,0
	.set	__base,__base+1
	.endr

#ifdef __ELF__
    /* "a" defines this section to be allocatable.  Without that, the binary objdump will strip this section */
	.section .rom_smpage_table, "a"
#else
    /* "a" is an unknown attribute for coff targets. */
	.section .rom_smpage_table
#endif
	.global SYM_NAME(page2)
SYM_NAME(page2):
	/*
	 * This page table breaks the LO 8MB down into 4KB pages
	 */
	.set	__base,0x0

	SL_SMPAGE_ENTRY __base,0,0,0,0,1,0 /* Physical address 0 (Flash boot code). */
	.set	__base,__base+1		   /* Read-only, cacheable, non-bufferable  */

	SL_SMPAGE_ENTRY __base,3,3,3,3,0,0 /* Physical address 0x1000 (Memory mapped registers). */
	.set	__base,__base+1		   /* Read-write, non-cacheable, non-bufferable		 */

	.rept	0x800 - 0x2
	SL_SMPAGE_ENTRY __base,0,0,0,0,1,0 /* Physical address 0x2000-0x800000 (remainder of flash1). */
	.set	__base,__base+1		   /* Read-only, cacheable, non-bufferable		      */
	.endr
#else
	.section .arm_page_table
	.global SYM_NAME(page1)
SYM_NAME(page1): .rept ARM_FIRST_LEVEL_PAGE_TABLE_SIZE
		 .byte 0
		 .endr
#endif /* XARET */
#endif /* MMU */

/*
 * Macro Definition for an EXCEPTION stack
 */
.macro EXCEPTION_STACK label, size
	.bss
	.global SYM_NAME(\label)
	.align 4
	.rept	\size
	.byte	0
	.endr
SYM_NAME(\label):
.endm


/*
 * Provide a stack for use by exception processing
 */
	EXCEPTION_STACK _ex_stack, 16*1024

/*
 * Provide a simple stack for use in each of the
 * exception modes.  These are pretty small since
 * all we do is immediately switch to the exception
 * stack
 */
#define TMP_STKSIZE 32
	EXCEPTION_STACK _abort_stack, TMP_STKSIZE
	EXCEPTION_STACK _address_err_stack, TMP_STKSIZE
	EXCEPTION_STACK _undefined_stack, TMP_STKSIZE
	EXCEPTION_STACK _irq_stack, TMP_STKSIZE
	EXCEPTION_STACK _fiq_stack, TMP_STKSIZE
