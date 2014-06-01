/*
 * ebsa-285.h -- Header for Intel(R) EBSA-285 Evaluation Board
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
#ifndef __EBSA_285_H__
#define __EBSA_285_H__ 1

#include <bsp/bsp.h>
#include <bsp/sa-110.h>
#include <bsp/cpu.h>

/*
 * EBSA-285 Soft I/O Register
 */
#define EBSA_285_SOFT_IO_REGISTER           REG32_PTR(0x40012000)

/*
 * EBSA-285 Soft I/O Register Bit Field definitions
 */
#define EBSA_285_SOFT_IO_TOGGLE             0x80
#define EBSA_285_SOFT_IO_RED_LED            0x04
#define EBSA_285_SOFT_IO_GREEN_LED          0x02
#define EBSA_285_SOFT_IO_AMBER_LED          0x01
#define EBSA_285_SOFT_IO_J9_9_10_MASK       0x40
#define EBSA_285_SOFT_IO_J9_11_12_MASK      0x20
#define EBSA_285_SOFT_IO_J9_13_14_MASK      0x10
#define EBSA_285_SOFT_IO_SWITCH_L_MASK      0x0F

#ifdef __ASSEMBLER__
/*
 * Use the following code for debugging by toggling a port pin
 */
.macro PORT_TOGGLE_DEBUG
        /*
         * Leave the following values in registers:
         *
         *    r0 = Soft IO Register
         *    r1 = LED_ON value
         *    r2 = LED_OFF value
         *    r3 = LED_DELAY value
         */
        ldr     r0, =EBSA_285_SOFT_IO_REGISTER
        ldr     r1, =(EBSA_285_SOFT_IO_RED_LED   | \
                      EBSA_285_SOFT_IO_GREEN_LED | \
                      EBSA_285_SOFT_IO_AMBER_LED)
        eor     r2, r1, IMM(7)
        ldr     r3, =0x1FFFF

        /*
         * Turn on the LED
         */
0:      strb  r1, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, IMM(1)
        bne     1b

        /*
         * Turn off the LED
         */
        strb   r2, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, IMM(1)
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
         *    r0 = Soft IO Register
         *    r1 = LED_ON value
         *    r2 = LED_OFF value
         *    r3 = LED_DELAY value
         */
        ldr     r0, =0x40012000
        ldr     r1, =0x07
        eor     r2, r1, #7
        ldr     r3, =0x1ffff

        /*
         * Turn on the LED
         */
0:      strb    r1, [r0]
        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, #1
        bne     1b

        /*
         * Turn off the LED
         */
        strb    r2, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, #1
        bne     1b

        /*
         * Repeat
         */
        b       0b")
#endif /* __ASSEMBLER__ */

#endif /* __EBSA_285_H__ */
