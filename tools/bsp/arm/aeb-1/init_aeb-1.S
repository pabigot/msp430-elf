/*
 * init_aeb-1.S -- Initialization for ARM(R) AEB-1 Evaluation board.
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
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
	.file "init_aeb-1.S"
        .title "Init Code for ARM(R) AEB-1 Evaluation Board"

#include __BOARD_HEADER__

#define ROM_SDR_VALUE       (LH77790A_MEM_SEG_SYS_PRIVILEGE_RO  | \
                             LH77790A_MEM_SEG_USER_PRIVILEGE_RO | \
                             LH77790A_MEM_SEG_NONCACHEABLE      | \
                             LH77790A_MEM_SEG_32BIT_MODE        | \
                             LH77790A_MEM_SEG_SELECT_BANK_0)
#define ROM_BCR_VALUE       (LH77790A_BANK_SRAM_BUS_SIZE_8_BITS | \
                             LH77790A_BANK_SRAM_WAIT_STATES_7   | \
                             LH77790A_BANK_SRAM_ECE_CE0_HIGH    | \
                             LH77790A_BANK_SRAM_ECE_CE0_LOW)
#define RAM_SDR_VALUE       (LH77790A_MEM_SEG_SYS_PRIVILEGE_RW  | \
                             LH77790A_MEM_SEG_USER_PRIVILEGE_RW | \
                             LH77790A_MEM_SEG_NONCACHEABLE      | \
                             LH77790A_MEM_SEG_32BIT_MODE        | \
                             LH77790A_MEM_SEG_SELECT_BANK_2)
#define RAM_BCR_VALUE       (LH77790A_BANK_SRAM_BUS_SIZE_16_BITS | \
                             LH77790A_BANK_SRAM_WAIT_STATES_1    | \
                             LH77790A_BANK_SRAM_ECE_CE1_HIGH     | \
                             LH77790A_BANK_SRAM_ECE_CE1_LOW)

	.text

        .align     4
FUNC_START _bsp_init_stack
        /*
         * Setup the ROM/FLASH
         */
        ldr     r0, =LH77790A_MEM_SEG_START0
        ldr     r1, =LH77790A_MEM_SEG_START_ADDRESS(256*1024)
        str     r1, [r0]

        ldr     r0, =LH77790A_MEM_SEG_STOP0
        ldr     r1, =LH77790A_MEM_SEG_STOP_ADDRESS(512*1024)
        str     r1, [r0]

        ldr     r0, =LH77790A_MEM_SEG_SDR0
        ldr     r1, =ROM_SDR_VALUE
        str     r1, [r0]

        ldr     r0, =LH77790A_BANK_CONFIG_BCR0
        ldr     r1, =ROM_BCR_VALUE
        str     r1, [r0]

        BOARD_BOOT_LED_DEBUG 1

        /*
         * Now we have ROM.
         *
         * Load the PC directly with the address of the next instruction
         * so that we start executing from ROM.
         */
        ldr     pc, =0f
0:

        BOARD_BOOT_LED_DEBUG 2

        /*
         * Setup the RAM
         */
        ldr     r0, =LH77790A_MEM_SEG_START2
        ldr     r1, =LH77790A_MEM_SEG_START_ADDRESS(0)
        str     r1, [r0]

        ldr     r0, =LH77790A_MEM_SEG_STOP2
        ldr     r1, =LH77790A_MEM_SEG_STOP_ADDRESS(128*1024)
        str     r1, [r0]

        ldr     r0, =LH77790A_MEM_SEG_SDR2
        ldr     r1, =RAM_SDR_VALUE
        str     r1, [r0]

        ldr     r0, =LH77790A_BANK_CONFIG_BCR2
        ldr     r1, =RAM_BCR_VALUE
        str     r1, [r0]

        BOARD_BOOT_LED_DEBUG 3

        /*
         * Save the Return Address
         */
        mov     r8, lr

        /*
         * Setup a temporary stack pointer pointing somewhere
         * in the unmapped RAM address space.
         */
        ldr     sp, =(RAM_ACTUAL_BASE + RAM_TOTAL_SIZE)

        /*
         * Setup the Page Tables
         */
        bl      SYM_NAME(_bsp_mmu_init)
     
        /*
         * Setup the real stack pointer pointing at the 
         * top of the remapped RAM address -- returned
         * in r0 by _bsp_mmu_init.
         */
        mov     sp, r0

        /*
         * Absolute address of 0: in r1.
         * This is so we can jump there after remapping
         * the ROM
         */
        ldr     r1, =0f

        /*
         * Now set the MMU Control Register to actually
         * enable the MMU, etc.
         */

        /*
         * Jump to the absolute address of the next instruction
         */
        mov     pc, r1
0:
        BOARD_BOOT_LED_DEBUG 4

        /*
         * Tell c_start where the top of ram is
         */
        mov     r0, sp

	ldr	r1, =LH77790A_CACHE_REGISTER
	mov	r2, IMM(1)
	str	r2, [r1]

        /*
         * Return
         */
        mov     pc, r8
FUNC_END _bsp_init_stack
