/*
 * iq80310.h -- Header for Intel(R) SA-220 Eval Board
 *
 * Copyright (c) 2000 Red Hat, Inc.
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
 */
#ifndef __IQ80310_H__
#define __IQ80310_H__ 1

#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     0xA0000000
#define RAM_VIRTUAL_BASE    0xA0000000
#define RAM_TOTAL_SIZE      (32 * 1024*1024) 

/*
 * Flash Definitions
 */
#define FLASH_ACTUAL_BASE   0x00000000
#define FLASH_VIRTUAL_BASE  0xA0000000
#define FLASH_TOTAL_SIZE    SZ_8M

/*
 * FLASH1 base is used for SRAM and some memory mapped IO.
 */
/* #define FLASH1_ACTUAL_BASE  0xB0000000 */
/* #define FLASH1_VIRTUAL_BASE 0xB0000000 */ 
/* #define FLASH1_TOTAL_SIZE   SZ_512K */

/*
 * 7-segment display addresses
 */
#ifdef __ASSEMBLER__
#define DISPLAY_LEFT	0xFE840000
#define DISPLAY_RIGHT	0xFE850000
#else
#define DISPLAY_LEFT	((volatile unsigned char *)0xFE840000)
#define DISPLAY_RIGHT	((volatile unsigned char *)0xFE850000)
#endif

/*
 * 7-segment encodings for the hex display
 */
#define DISPLAY_0	0xc0
#define DISPLAY_1	0xf9
#define DISPLAY_2	0xa4
#define DISPLAY_3	0xb0
#define DISPLAY_4	0x99
#define DISPLAY_5	0x92
#define DISPLAY_6	0x82
#define DISPLAY_7	0xF8
#define	DISPLAY_8	0x80
#define DISPLAY_9	0x90
#define DISPLAY_A	0x88
#define DISPLAY_B	0x83
#define DISPLAY_C	0xa7
#define DISPLAY_D	0xa1
#define DISPLAY_E	0x86
#define DISPLAY_F	0x8e

#ifdef __ASSEMBLER__
.macro BOARD_BOOT_LED_DEBUG num
	ldr	r0, =DISPLAY_LEFT		/* display left digit */
	ldr	r1, =\num
	str	r1, [r0]
	ldr	r0, =DISPLAY_RIGHT
	ldr	r1, =\num			/* display right digit */
	str	r1, [r0]
.endm
.macro PORT_TOGGLE_DEBUG
        /*
         * Leave the following values in registers:
         *
         *    r0 = Left Display Register
         *    r1 = Right Display Register
         *    r2 = LED_ON value
         *    r3 = LED_OFF value
         *    r4 = LED_DELAY value
         */
        ldr     r0, =DISPLAY_LEFT
        ldr     r1, =DISPLAY_RIGHT
        ldr     r2, =~0
        ldr     r3, =0
        ldr     r4, =0x1FFFFFF

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
extern void flash_led(int, int);
#define BOARD_BOOT_LED_DEBUG(num) {*DISPLAY_LEFT = num; *DISPLAY_RIGHT = num;}

#define BOARD_BOOT_STAGE_1() BOARD_BOOT_LED_DEBUG(DISPLAY_5)
#define BOARD_BOOT_STAGE_2() BOARD_BOOT_LED_DEBUG(DISPLAY_6)
#define BOARD_BOOT_STAGE_3() BOARD_BOOT_LED_DEBUG(DISPLAY_7)

#define PORT_TOGGLE_DEBUG() {                                 \
    while (1)                                                 \
    {                                                         \
        *DISPLAY_LEFT = ~0;                                   \
        *DISPLAY_RIGHT = ~0;                                  \
        {                                                     \
            volatile int i = 0xffffff;                        \
            while (--i) ;                                     \
        }                                                     \
        *DISPLAY_LEFT = 0;                                    \
        *DISPLAY_RIGHT = 0;                                   \
        {                                                     \
            volatile int i = 0xffffff;                        \
            while (--i) ;                                     \
        }                                                     \
    }                                                         \
}

#define DEBUG_HEX_DISPLAY(left, right) {*DISPLAY_LEFT = (left); *DISPLAY_RIGHT = (right);}
#endif /* __ASSEMBLER__ */

/*
 * These definitions correspond to the IQ80310 PAL
 */
#define IQ80310_XINT3_MASK                      ((volatile unsigned char*)0xFE860000)
#define IQ80310_XINT3_TIMER_INTERRUPT           0x01
#define IQ80310_XINT3_ETHERNET_INTERRUPT        0x02
#define IQ80310_XINT3_UART1_INTERRUPT           0x04
#define IQ80310_XINT3_UART2_INTERRUPT           0x08
#define IQ80310_XINT3_SECONDARY_PCI_INTERRUPT   0x10

#endif /* __IQ80310_H__ */
