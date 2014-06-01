/*
 * sa1100dp.h -- Header for Intel(R) SA-1100 Microprocessor Evaluation Platform
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
 * Other Brands and Trademarks are the property of their respective owners.
 */
#ifndef __SA1100DP_H__
#define __SA1100DP_H__ 1

#include <bsp/bsp.h>
#include <bsp/sa-1100.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * ROM Definitions
 */
#define ROM1_ACTUAL_BASE    SA1100_ROM_BANK0_BASE
#define ROM2_ACTUAL_BASE    SA1100_ROM_BANK1_BASE
#define ROM3_ACTUAL_BASE    SA1100_ROM_BANK2_BASE
#define ROM4_ACTUAL_BASE    SA1100_ROM_BANK3_BASE
#define ROM1_BANK_SIZE      SZ_128K
#define ROM2_BANK_SIZE      0
#define ROM3_BANK_SIZE      0
#define ROM4_BANK_SIZE      0
#define ROM1_VIRTUAL_BASE   0x50000000
#define ROM2_VIRTUAL_BASE   0xffffffff    /* unmapped */
#define ROM3_VIRTUAL_BASE   0xffffffff    /* unmapped */
#define ROM4_VIRTUAL_BASE   0xffffffff    /* unmapped */

#define ROM_ACTUAL_BASE     (ROM1_ACTUAL_BASE)
#define ROM_TOTAL_SIZE      (ROM1_BANK_SIZE)
#define ROM_VIRTUAL_BASE    (ROM1_VIRTUAL_BASE)

/*
 * RAM Definitions
 */
#define RAM1_ACTUAL_BASE    SA1100_RAM_BANK0_BASE
#define RAM2_ACTUAL_BASE    SA1100_RAM_BANK1_BASE
#define RAM3_ACTUAL_BASE    SA1100_RAM_BANK2_BASE
#define RAM4_ACTUAL_BASE    SA1100_RAM_BANK3_BASE
#define RAM1_BANK_SIZE      SZ_4M
#define RAM2_BANK_SIZE      SZ_4M
#define RAM3_BANK_SIZE      SZ_4M
#define RAM4_BANK_SIZE      SZ_4M
#define RAM1_VIRTUAL_BASE   0x00000000
#define RAM2_VIRTUAL_BASE   (RAM1_VIRTUAL_BASE + RAM1_BANK_SIZE)
#define RAM3_VIRTUAL_BASE   (RAM2_VIRTUAL_BASE + RAM2_BANK_SIZE)
#define RAM4_VIRTUAL_BASE   (RAM3_VIRTUAL_BASE + RAM3_BANK_SIZE)

#define RAM_ACTUAL_BASE     (RAM1_ACTUAL_BASE)
#define RAM_TOTAL_SIZE      (RAM1_BANK_SIZE + RAM2_BANK_SIZE + RAM3_BANK_SIZE + RAM4_BANK_SIZE)
#define RAM_VIRTUAL_BASE    (RAM1_VIRTUAL_BASE)

/*
 * Flash Definitions
 */
#define FLASH1_ACTUAL_BASE  0x08000000
#define FLASH1_VIRTUAL_BASE 0x08000000
#define FLASH1_BANK_SIZE    SZ_4M

#define FLASH_ACTUAL_BASE   (FLASH1_ACTUAL_BASE)
#define FLASH_TOTAL_SIZE    (FLASH1_BANK_SIZE)
#define FLASH_VIRTUAL_BASE  (FLASH1_VIRTUAL_BASE)

/*
 * Sa1100dp Board Register definitions
 */
#define SA1100DP_DISCRETE_LED_SET_REGISTER              SA1100_GPIO_PIN_OUTPUT_SET_REGISTER
#define SA1100DP_DISCRETE_LED_CLEAR_REGISTER            SA1100_GPIO_PIN_OUTPUT_CLEAR_REGISTER
#define SA1100DP_DISCRETE_LED_DIR_REGISTER              SA1100_GPIO_PIN_DIR_REGISTER
#define SA1100DP_HEX_LED_CONTROL_REGISTER               SA1100_MCP_CONTROL_0
#define SA1100DP_HEX_LED_STATUS_REGISTER                SA1100_MCP_STATUS
#define SA1100DP_HEX_LED_REGISTER                       SA1100_MCP_DATA_2

/*
 * Sa1100dp Board LED definitions
 */
#define GREEN_LED_0                                   BIT8
#define GREEN_LED_1                                   BIT9
#define RED_LED                                       BIT20
#define ALL_LEDS                                      (GREEN_LED_0 + GREEN_LED_1 + RED_LED)
#define HEX_LED                                       0

/*
 * Sa1100dp Board Switch Definitions
 */
#define SA1100DP_RAM_CTL_DRAM                           0x0
#define SA1100DP_RAM_CTL_SRAM                           0x2
#define SA1100DP_RAM_CTL_MASK                           0x2

#ifdef __ASSEMBLER__
/*
 * Use the following code for debugging by toggling a port pin
 */
.macro PORT_TOGGLE_DEBUG
        /*
         * Leave the following values in registers:
         *
         *    r0 = GPIO Set Register
         *    r1 = GPIO Clear Register
         *    r2 = LED value
         *    r3 = LED_DELAY value
         */
        ldr     r0, =SA1100DP_DISCRETE_LED_SET_REGISTER
        ldr     r1, =SA1100DP_DISCRETE_LED_CLEAR_REGISTER
        ldr     r2, =(BIT27 | ALL_LEDS)
        ldr     r3, =0x7FFFFF

        /*
         * Set the GPIO pins to be output
         */
        ldr     r4, =SA1100DP_DISCRETE_LED_DIR_REGISTER
        str     r2, [r4]

        /*
         * turn off all the LEDs
         */
0:      str     r2, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, IMM(1)
        bne     1b

        /*
         * turn on all the LEDs
         */
        str     r2, [r1]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, IMM(1)
        bne     1b

        b       0b
.endm
#else
#define PORT_TOGGLE_DEBUG()    asm volatile ("
        /*
         * Leave the following values in registers:
         *
         *    r0 = GPIO Set Register
         *    r1 = GPIO Clear Register
         *    r2 = LED value
         *    r3 = LED_DELAY value
         */
        ldr     r0, =0x90040008
        ldr     r1, =0x9004000C
        ldr     r2, =0x08100300
        ldr     r3, =0x7FFFFF

        /*
         * Set the GPIO pins to be output
         */
        ldr     r4, =0x90040004
        str     r2, [r4]

        /*
         * turn off all the LEDs
         */
0:      str     r2, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, #1
        bne     1b

        /*
         * turn on all the LEDs
         */
        str     r2, [r1]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, #1
        bne     1b

        b       0b
");
#endif /* __ASSEMBLER__ */

#ifdef __ASSEMBLER__
.macro BOARD_BOOT_LED_DEBUG num
        ldr     r0, =SA1100DP_HEX_LED_REGISTER
        ldr     r1, =(0x38010 | (~(\num) & 0xF))
        str     r1, [r0]
.endm
#else
extern void flash_led(int, int);
#define BOARD_BOOT_LED_DEBUG(num) flash_led((num), HEX_LED)

#define BOARD_BOOT_STAGE_1() BOARD_BOOT_LED_DEBUG(5)
#define BOARD_BOOT_STAGE_2() BOARD_BOOT_LED_DEBUG(6)
#define BOARD_BOOT_STAGE_3() BOARD_BOOT_LED_DEBUG(7)
#endif /* __ASSEMBLER__ */


#endif /* __SA1100DP_H__ */
