/*
 * sa-iop.h -- Header for Intel(R) SA-IOP Evaluation Board
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
#ifndef __SA_IOP_H__
#define __SA_IOP_H__ 1

#include <bsp/bsp.h>
#include <bsp/sa-110.h>
#include <bsp/cpu.h>

/*
 * SA-IOP Soft I/O Register
 */
#define SA_IOP_SOFT_IO_REGISTER           REG32_PTR(0x40012000)

/*
 * SA-IOP Soft I/O Register Bit Field definitions
 */
#define SA_IOP_SOFT_IO_TOGGLE             0x80
#define SA_IOP_SOFT_IO_LED(x)             ((((x) >=0) && (x <=6)) ? (1 << (x)) : (0))
#define SA_IOP_SOFT_IO_J9_9_10_MASK       0x40
#define SA_IOP_SOFT_IO_J9_11_12_MASK      0x20
#define SA_IOP_SOFT_IO_J9_13_14_MASK      0x10
#define SA_IOP_SOFT_IO_SWITCH_L_MASK      0x0F

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
        ldr     r0, =SA_IOP_SOFT_IO_REGISTER
        ldr     r1, =0x7F
        mvn     r2, r1
        ldr     r3, =0x3FFFFF

        /*
         * Turn on the LED
         */
0:      strb    r1, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, IMM(1)
        bne     1b

        /*
         * Turn off the LED
         */
        strb    r2, [r0]

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
        ldr     r1, =0x7F
        mvn     r2, r1
        ldr     r3, =0x3FFFFF

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

#endif /* __SA_IOP_H__ */
