/*
 * aeb-1.h -- Header for ARM(R) AEB-1 Eval Board
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
 */
#ifndef __AEB_1_H__
#define __AEB_1_H__ 1

#include <bsp/bsp.h>
#include <bsp/lh77790a.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     0x00000000
#define RAM_TOTAL_SIZE      SZ_128K
#define RAM_VIRTUAL_BASE    0x00000000

/*
 * Flash Definitions
 */
#define FLASH_ACTUAL_BASE   0x00040000
#define FLASH_TOTAL_SIZE    SZ_256K
#define FLASH_VIRTUAL_BASE  0x00040000

/*
 * AEB-1 Board LED definitions
 */
#define AEB_1_LED_D1                       LH77790A_PORT_C_BIT_7
#define AEB_1_LED_D2                       LH77790A_PORT_C_BIT_6
#define AEB_1_LED_D3                       LH77790A_PORT_C_BIT_5
#define AEB_1_LED_D4                       LH77790A_PORT_C_BIT_4

#ifdef __ASSEMBLER__
/*
 * Use the following code for debugging by toggling a port pin
 */
.macro PORT_TOGGLE_DEBUG
        /*
         * Make sure the Port C is in Output mode
         */
        ldr     r0, =LH77790A_PORT_CONTROL_REGISTER
        ldr     r1, =(LH77790A_PORT_CTL_MODE_SELECTION     | \
                      LH77790A_PORT_C_4_7_DIRECTION_OUTPUT | \
                      LH77790A_PORT_C_0_3_DIRECTION_OUTPUT)
        strb    r1, [r0]

        /*
         * Write to the PORT C register
         */
        ldr     r0, =LH77790A_PORT_C

        /*
         * Turn on the LED's
         */
0:      ldr     r1, =0xF0
        strb    r1, [r0]

        /*
         * Delay a while
         */
        ldr     r2, =0xFFFF
1:      subs    r2, r2, IMM(1)
        bgt     1b

        /*
         * Turn off the LED's
         */
        ldr     r1, =0x00
        strb    r1, [r0]

        /*
         * Delay a while
         */
        ldr     r2, =0xffff
1:      subs    r2, r2, IMM(1)
        bgt     1b

        b       0b
.endm
#else

#define PORT_TOGGLE_DEBUG()  asm volatile ("\n\
        /* Make sure the Port C is in Output mode.  */\n\
        ldr     %r0, =0xFFFF1C0C\n\
        ldr     %r1, =0x80\n\
        strb    %r1, [%r0]\n\
\n\
        /* Write to the PORT C register.  */\n\
        ldr     %r0, =0xFFFF1C08\n\
\n\
        /* Turn on the LED's.  */\n\
0:      ldr     %r1, =0xF0\n\
        strb    %r1, [%r0]\n\
\n\
        /* Delay a while.  */\n\
        ldr     %r2, =0xffff\n\
1:      subs    %r2, %r2, #1\n\
        bgt     1b\n\
\n\
        /* Turn off the LED's.  */\n\
        ldr     %r1, =0x00\n\
        strb    %r1, [%r0]\n\
\n\
        /* Delay a while.  */\n\
        ldr     %r2, =0xffff\n\
1:      subs    %r2, %r2, #1\n\
        bgt     1b\n\
\n\
        b       0b");
#endif /* __ASSEMBLER__ */

#define UART_BASE_0 LH77790A_UART_BASE_1
#define UART_BASE_1 LH77790A_UART_BASE_0

#ifdef __ASSEMBLER__
.macro BOARD_BOOT_LED_DEBUG num
.endm
#else
extern void flash_led(int, int);
#define BOARD_BOOT_LED_DEBUG(num)

#define BOARD_BOOT_STAGE_1() BOARD_BOOT_LED_DEBUG(5)
#define BOARD_BOOT_STAGE_2() BOARD_BOOT_LED_DEBUG(6)
#define BOARD_BOOT_STAGE_3() BOARD_BOOT_LED_DEBUG(7)
#endif /* __ASSEMBLER__ */

#endif /* __AEB_1_H__ */
