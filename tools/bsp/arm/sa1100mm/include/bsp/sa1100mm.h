/*
 * sa1100mm.h -- Header for Intel(R) SA-1100 Multimedia Development Board
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
#ifndef __SA1100MM_H__
#define __SA1100MM_H__ 1

#include <bsp/bsp.h>
#include <bsp/sa-1100.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * ROM Definitions
 */
#define ROM_ACTUAL_BASE     SA1100_ROM_BANK0_BASE
#define ROM_TOTAL_SIZE      SZ_128K
#define ROM_VIRTUAL_BASE    0x50000000

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     SA1100_RAM_BANK0_BASE
#define RAM_TOTAL_SIZE      SZ_8M
#define RAM_VIRTUAL_BASE    0x00000000

/*
 * Flash Definitions
 */
#define FLASH_ACTUAL_BASE   0x08000000
#define FLASH_TOTAL_SIZE    SZ_4M
#define FLASH_VIRTUAL_BASE  0x08000000

/*
 * SA1100 Multimedia Board Register definitions
 */
#define SA1100MM_XBUS_CONTROL_REGISTER          REG32_PTR(0x18400000)
#define SA1100MM_SYSTEM_RESET_REGISTER          REG32_PTR(0x18800000)
#define SA1100MM_VIDEO_INPUT_MODE_REGISTER      REG32_PTR(0x18800002)
#define SA1100MM_KEYPAD_IO_REGISTER             REG32_PTR(0x18800004)
#define SA1100MM_DISCRETE_LED_REGISTER          REG32_PTR(0x18800006)
#define SA1100MM_HEX_LED_REGISTER               REG32_PTR(0x18C00000)
#define SA1100MM_SWITCH_REGISTER                REG32_PTR(0x18C00002)
#define SA1100MM_XBUS_INTERRUPT_REASON_REGISTER REG32_PTR(0x18C00004)
#define SA1100MM_REVISION_REGISTER              REG32_PTR(0x18C00006)

/*
 * Hex LED Register Bit Field Definitions
 */
#define SA1100MM_HEX_DATA_MASK                  0x0F
#define SA1100MM_HEX_LED_0_STROBE               0x10
#define SA1100MM_HEX_LED_1_STROBE               0x20
#define SA1100MM_HEX_LED_BOTH_STROBES           (SA1100MM_HEX_LED_0_STROBE | \
                                                 SA1100MM_HEX_LED_1_STROBE)
#define SA1100MM_HEX_VID_CAP_ENABLE             0x40
#define SA1100MM_HEX_ENABLE_INTERRUPTS          0x80

#ifdef __ASSEMBLER__
/*
 * Use the following code for debug by flashing some LED's
 */
.macro PORT_TOGGLE_DEBUG
        /*
         * Leave the following values in registers:
         *
         *    r0 = Discrete LED Register
         *    r1 = Keypad IO Register
         *    r2 = LED_ON value
         *    r3 = LED_OFF value
         *    r4 = LED_DELAY value
         */
        ldr     r0, =SA1100MM_DISCRETE_LED_REGISTER
        ldr     r1, =SA1100MM_KEYPAD_IO_REGISTER
        ldr     r2, =0
        mvn     r3, r2
        ldr     r4, =0x7FFFFF

0:
        /*
         * Turn the LED's on
         */
  	strh	r2, [r0]
	strh	r2, [r1]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, IMM(1)
        bne     1b

        /*
         * Turn the LED's off
         */
  	strh	r3, [r0]
	strh	r3, [r1]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, IMM(1)
        bne     1b

        /*
         * Repeat
         */
        b       0b
.endm
#else
#define PORT_TOGGLE_DEBUG()    asm volatile ("
        /*
         * Leave the following values in registers:
         *
         *    r0 = Discrete LED Register
         *    r1 = Keypad IO Register
         *    r2 = LED_ON value
         *    r3 = LED_OFF value
         *    r4 = LED_DELAY value
         */
        ldr     r0, =0x18800006
        ldr     r1, =0x18800004
        ldr     r2, =0
        mvn     r3, r2
        ldr     r4, =0x7FFFFF

0:
        /*
         * Turn the LED's on
         */
  	strh	r2, [r0]
	strh	r2, [r1]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, #1
        bne     1b

        /*
         * Turn the LED's off
         */
  	strh	r3, [r0]
	strh	r3, [r1]

        /*
         * Delay a while
         */
        mov     r5, r4
1:      subs    r5, r5, #1
        bne     1b

        /*
         * Repeat
         */
        b       0b");
#endif /* __ASSEMBLER__ */

#ifdef __ASSEMBLER__

.macro BOARD_BOOT_LED_DEBUG num
            ldr r0, =SA1100MM_HEX_LED_REGISTER

            ldr r1, =(\num | SA1100MM_HEX_LED_BOTH_STROBES)
            str r1, [r0]
            
            ldr r1, =(\num & ~SA1100MM_HEX_LED_0_STROBE)
            str r1, [r0]

            ldr r1, =(\num | SA1100MM_HEX_LED_BOTH_STROBES)
            str r1, [r0]
.endm
#else
extern void flash_led(int, int);
#define BOARD_BOOT_LED_DEBUG(num) flash_led((num), 8)

#define BOARD_BOOT_STAGE_1() BOARD_BOOT_LED_DEBUG(5)
#define BOARD_BOOT_STAGE_2() BOARD_BOOT_LED_DEBUG(6)
#define BOARD_BOOT_STAGE_3() BOARD_BOOT_LED_DEBUG(7)
#endif /* __ASSEMBLER__ */


#endif /* __SA1100MM_H__ */
