/*
 * init_ebsa-285.S -- Initialization for Intel(R) EBSA-285 Evaluation Board
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
 *
 * Intel is a Registered Trademark of Intel Corporation.
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
	.file "init_ebsa-285.S"
        .title "Init Code for Intel(R) EBSA-285 Evaluation Board"

#include __BOARD_HEADER__
        
#define MMU_ENABLE_VALUE               (ARM_MMU_ENABLED                | \
			                ARM_ADDRESS_FAULT_DISABLED     | \
				        ARM_DATA_CACHE_ENABLED         | \
                                        ARM_WRITE_BUFFER_ENABLED       | \
                                        ARM_LITTLE_ENDIAN              | \
                                        ARM_ACCESS_CHECKS_NONE         | \
                                        ARM_INSTRUCTION_CACHE_ENABLED  | \
                                        ARM_VIRTUAL_IVR_BASE_00000000)
	
	.text

        .align     4

FUNC_START _bsp_init_stack

	/*
	 * Disable ROM address alias.
	 * First, force pc to "normal" address, then
	 * perform a bus write to disable the alias.
	 */
	ldr	pc, =2f		/* load absolute address of 2f */
    2:
        str     r0, [r0, -r0]	/* disable the ROM alias */
	
	/* Allow clock switching */
        ldr     r0, =0
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                c15,               \
                c1,       \
                2
	
	/* set up XBus so we can read switch and write to LEDs */
	ldr	r0, =SA110_CONTROL
	ldr	r1, =0x64aa0000
        str     r1, [r0]
	ldr	r0, =SA110_XBUS_CYCLE_ARBITER
	ldr	r1, =0x100016db
        str     r1, [r0]
	ldr	r0, =SA110_XBUS_IO_STROBE_MASK
	ldr	r1, =0xfcfcfcfc
        str     r1, [r0]
	
	/* Save lr and call mem init code. */
	mov	r10, lr
	bl	SYM_NAME(__mem285_init)
	mov	lr, r10
	
	mov	sp, r0
	sub	sp, sp, IMM(32)
	str	r0, [sp]
	
	/****************************************************************
	 *  Basic PCI setup.
	 ****************************************************************/
	ldr	r0, =SA110_CONTROL_STATUS_BASE

	/* Disable PCI Outbound interrupts */
	ldr	r1, =12
	str	r1, [r0, IMM(SA110_OUT_INT_MASK_o)]

	/* Disable Doorbells */
	ldr	r1, =0
	str	r1, [r0, IMM(SA110_DOORBELL_PCI_MASK_o)]
	str	r1, [r0, IMM(SA110_DOORBELL_SA_MASK_o)]

	/* Map high PCI address bits to 0 */
	str	r1, [r0, IMM(SA110_PCI_ADDR_EXT_o)]

	/* Interrupt ID to 1 */
	ldr	r1, =0x100
	orr	r1, r1, IMM(1)
	str	r1, [r0, IMM(SA110_PCI_INT_LINE_o)]
	
	/* Remove PCI_reset */
	ldr	r1, [r0, IMM(SA110_CONTROL_o)]
	orr	r1, r1, IMM(0x200)
	str	r1, [r0, IMM(SA110_CONTROL_o)]

	/* Open a 2MB window */
	ldr	r1, =0x1c0000
	str	r1,[r0, IMM(SA110_SDRAM_BASE_ADDRESS_MASK_o)]
	ldr	r1, =0xe00000
	str	r1,[r0, IMM(SA110_SDRAM_BASE_ADDRESS_OFFSET_o)]
	
	/* Don't init PCI if central function or standalone bit is set */
	ldr	r1, [r0, IMM(SA110_CONTROL_o)]
	tst	r1, IMM(SA110_CONTROL_CFN)
	beq	1f

	ldr	r1, =0x40012000
	ldr	r1, [r1]
	tst	r1, IMM(0x40)
	bne	1f

	/* Don't respond to any commands */
	ldr	r1, =0
	str	r1, [r0, IMM(4)]  /* PCI command register */
	str	r1, [r0, IMM(0x18)]  /* PCI DRAM BAR */
	ldr	r1, =0x40000000
	str	r1, [r0, IMM(0x10)]  /* PCI MEM BAR */
	ldr	r1, =0xf000
	str	r1, [r0, IMM(0x14)]  /* PCI IO BAR */

	/* respond to I/O space & Memory transactions. */
	ldr	r1, =0x17
	str	r1, [r0, IMM(4)]     /* PCI command register */
    1:
	/* Signal PCI_init_complete */
	ldr	r1, [r0, IMM(SA110_CONTROL_o)]
	orr	r1, r1, IMM(SA110_CONTROL_INIT_COMPLETE)
	str	r1, [r0, IMM(SA110_CONTROL_o)]
	
        /*
         * Save the Return Address
         */
        mov     r8, lr

	/* Init page table. Pass sdram size. */
	ldr	r0, [sp]
	bl	SYM_NAME(_bsp_mmu_init)
	
        /*
         * Now set the MMU Control Register to actually
         * enable the MMU, etc.
	 * No need to do anything special with pc as
	 * all memory is direct mapped.
         */
        ldr     r0, =MMU_ENABLE_VALUE
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_CONTROL_REGISTER,               \
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE
	nop
	nop
	nop

	/*
	 * Get RAM size from stack and pass as argument. The
	 * return address is set up to send us to a C function.
	 */
	ldr	r0, [sp]
	
        mov     pc, r8
FUNC_END _bsp_init_stack
