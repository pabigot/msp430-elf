/*
 * init_sa1100mm.S -- Initialization for 
 *                             Intel(R) SA-1100 Multimedia Development Board
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
	.file "init_sa1100mm.S"
        .title "Init Code for Intel(R) SA-1100 Multimedia Development Board"

#include __BOARD_HEADER__
#include <bsp/defs.h>

#define ALL_LEDS_OFF           0xF

#define DRAM_CONFIG_VALUE      (SA1100_DRAM_REFRESH_INTERVAL(312)              | \
                                SA1100_DATA_INPUT_LATCH_CAS_PLUS_THREE         | \
                                SA1100_DRAM_CAS_BEFORE_RAS(5)                  | \
                                SA1100_DRAM_RAS_PRECHARGE(4)                   | \
                                SA1100_DRAM_CLOCK_CPU_CLOCK                    | \
                                SA1100_DRAM_ROW_ADDRESS_BITS_11                | \
                                SA1100_DRAM_BANK_0_ENABLED                     | \
                                SA1100_DRAM_BANK_1_DISABLED                    | \
                                SA1100_DRAM_BANK_2_DISABLED                    | \
                                SA1100_DRAM_BANK_3_DISABLED)

#define DRAM_CAS0_WAVEFORM     0xF0F0F00F
#define DRAM_CAS1_WAVEFORM     0XF0F0F0F0
#define DRAM_CAS2_WAVEFORM     0xFFFFFFF0

#define BANK_0_CONTROL_VALUE   SA1100_STATIC_ROM_TYPE_FLASH                    | \
                               SA1100_STATIC_ROM_BUS_WIDTH_16_BITS             | \
                               SA1100_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA1100_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA1100_STATIC_ROM_RECOVERY(0x7)
#define BANK_1_CONTROL_VALUE   SA1100_STATIC_ROM_TYPE_FLASH                    | \
                               SA1100_STATIC_ROM_BUS_WIDTH_32_BITS             | \
                               SA1100_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA1100_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA1100_STATIC_ROM_RECOVERY(0x7)
#define STATIC_CONTROL_0_VALUE (SA1100_STATIC_ROM_BANK_0(BANK_0_CONTROL_VALUE) | \
                                SA1100_STATIC_ROM_BANK_1(BANK_1_CONTROL_VALUE))

#define BANK_2_CONTROL_VALUE   SA1100_STATIC_ROM_TYPE_FLASH                    | \
                               SA1100_STATIC_ROM_BUS_WIDTH_16_BITS             | \
                               SA1100_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA1100_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA1100_STATIC_ROM_RECOVERY(0x7)
#define BANK_3_CONTROL_VALUE   SA1100_STATIC_ROM_TYPE_FLASH                    | \
                               SA1100_STATIC_ROM_BUS_WIDTH_32_BITS             | \
                               SA1100_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA1100_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA1100_STATIC_ROM_RECOVERY(0x7)
#define STATIC_CONTROL_1_VALUE (SA1100_STATIC_ROM_BANK_2(BANK_2_CONTROL_VALUE) | \
                                SA1100_STATIC_ROM_BANK_3(BANK_3_CONTROL_VALUE))

#define MMU_ENABLE_VALUE       (ARM_MMU_ENABLED                                | \
                                ARM_ADDRESS_FAULT_DISABLED                     | \
                                ARM_DATA_CACHE_ENABLED                         | \
                                ARM_WRITE_BUFFER_ENABLED                       | \
                                ARM_LITTLE_ENDIAN                              | \
                                ARM_ACCESS_CHECKS_NONE                         | \
                                ARM_INSTRUCTION_CACHE_ENABLED                  | \
                                ARM_VIRTUAL_IVR_BASE_00000000)

	.text

        .align     4
FUNC_START _bsp_init_stack
        /*
         * Initialize the Discrete LED's to off
         */
        ldr     r1, =ALL_LEDS_OFF
        ldr     r0, =SA1100MM_DISCRETE_LED_REGISTER
        str     r1, [r0]
        ldr     r0, =SA1100MM_KEYPAD_IO_REGISTER
        str     r1, [r0]

        BOARD_BOOT_LED_DEBUG 1

        /*
         * Set the speed on the PLL to 200MHz
         */
	ldr	r0, =SA1100_PWR_MGR_PLL_CONFIG
	ldr	r1, =SA1100_CLOCK_SPEED_206_4_MHz
	str	r1, [r0]

        /*
         * Initialize the DRAM Controller
         */
	ldr     r0, =SA1100_DRAM_CONFIGURATION
	ldr     r1, =DRAM_CONFIG_VALUE
	str     r1, [r0]
                
	ldr     r0, =SA1100_DRAM_CAS_0
	ldr     r1, =DRAM_CAS0_WAVEFORM
	str     r1, [r0]
                
	ldr     r0, =SA1100_DRAM_CAS_1
	ldr     r1, =DRAM_CAS1_WAVEFORM
	str     r1, [r0]
                
	ldr     r0, =SA1100_DRAM_CAS_2
	ldr     r1, =DRAM_CAS2_WAVEFORM
	str     r1, [r0]
                
	ldr     r0, =SA1100_STATIC_CONTROL_0

        /*
         * Get the reset ROM setup
         */
	ldr     r1, [r0]

        /*
         * Get the 16/32 bit setting to merge into the appropriate
         * register values later on.
         */
	and     r1, r1, IMM(SA1100_STATIC_ROM_BUS_WIDTH_MASK)

        /*
         * MSC0 - bank 0 ROM, bank 1 FLASH
         */
	ldr     r2, =STATIC_CONTROL_0_VALUE
	orr     r1, r1, r2

	str     r1, [r0]

        /*
         * MSC1 - bank 2 SRAM, bank 3 REG
         */
	ldr     r0, =SA1100_STATIC_CONTROL_1
	ldr     r1, =STATIC_CONTROL_1_VALUE
	str     r1, [r0]

        /*
         * Delay to let the DRAM warm up
         */
	ldr     r0, =0x200
0:	subs    r0, r0, IMM(1)
	bne     0b

        BOARD_BOOT_LED_DEBUG 2

        /*
         * Save the Return Address
         */
        mov     r8, lr

        /*
         * Setup a temporary stack pointer pointing somewhere
         * in the unmapped RAM address space.
         */
        ldr     sp, =(RAM_ACTUAL_BASE + SZ_512K)

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

        BOARD_BOOT_LED_DEBUG 3

        /*
         * Absolute address of 0 in r1.
         * This is so we can jump there after remapping
         * the ROM
         */
        ldr     r1, =0f

        /*
         * Now set the MMU Control Register to actually
         * enable the MMU, etc.
         */
        ldr     r0, =MMU_ENABLE_VALUE
        mcr     ARM_CACHE_COPROCESSOR_NUM,          \
                ARM_COPROCESSOR_OPCODE_DONT_CARE,   \
                r0,                                 \
                ARM_CONTROL_REGISTER,               \
                ARM_COPROCESSOR_RM_DONT_CARE,       \
                ARM_COPROCESSOR_OPCODE_DONT_CARE

        /*
         * Jump to the absolute address of the next instruction
         */
        mov     pc, r1

0:
        /*
         * Tell c_start where the top of ram is
         */
        BOARD_BOOT_LED_DEBUG 4
        mov     r0, sp

        /*
         * Return
         */
        mov     pc, r8
FUNC_END _bsp_init_stack
