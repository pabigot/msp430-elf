/*
 * init_cma222.S -- Initialization for Cogent CMA222 DB/ CMA110 MB.
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
	.file "init_cma222.S"
        .title "Init Code for Cogent(R) CMA222/CMA110 Evaluation Board"

#include <bsp/cma222.h>

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
	ldr	r0, =0x0e400000
        str     r0, [r0]	/* disable the ROM alias */
	
	ldr     r0, =(8*1024*1024)
	mov	sp, r0
	sub	sp, sp, IMM(32)
	str	r0, [sp]
	
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
