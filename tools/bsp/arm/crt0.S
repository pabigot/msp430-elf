/*
 * crt0.S -- startup file for ARM(R).
 *
 * Copyright (c) 1995, 1996, 1997, 1998, 1999 Cygnus Support
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

#include <bsp/cpu.h>

#define TEST_SINGLESTEP 0

    .file   "bsp-crt0.S"
    .text
     /*
      * Setup the assembly entry point.
      */
FUNC_START start
#if TEST_SINGLESTEP
.macro CLEAR_CARRY_BIT
     mrs     r2, cpsr
     bic     r2, r2, IMM(0x20000000)
     msr     cpsr, r2
.endm

.macro SET_CARRY_BIT
     mrs     r2, cpsr
     orr     r2, r2, IMM(0x20000000)
     msr     cpsr, r2
.endm

     ldr     r0, =0
     mov     r1, r0
     cmps    r0, r1

     /*
      * Test load/store multiple w/ lots of registers
      */
     ldr     lr, =0f
     stmfd   sp!, {r0-r12, lr}
     ldmfd   sp!, {r0-r12, pc}
     nop

0:
     ldr     lr, =1f
     stmfa   sp!, {r0-r12, lr}
     ldmfa   sp!, {r0-r12, pc}
     nop

1:
     ldr     lr, =2f
     stmed   sp!, {r0-r12, lr}
     ldmed   sp!, {r0-r12, pc}
     nop

2:
     ldr     lr, =3f
     stmea   sp!, {r0-r12, lr}
     ldmea   sp!, {r0-r12, pc}
     nop

3:

     /*
      * Test load/store multiple w/ a few registers
      */
     ldr     lr, =0f
     stmfd   sp!, {r0, r4, r12, lr}
     ldmfd   sp!, {r0, r4, r12, pc}
     nop

0:
     ldr     lr, =1f
     stmfa   sp!, {r0, r4, r12, lr}
     ldmfa   sp!, {r0, r4, r12, pc}
     nop

1:
     ldr     lr, =2f
     stmed   sp!, {r0, r4, r12, lr}
     ldmed   sp!, {r0, r4, r12, pc}
     nop

2:
     ldr     lr, =3f
     stmea   sp!, {r0, r4, r12, lr}
     ldmea   sp!, {r0, r4, r12, pc}
     nop

3:

     /*
      * Test load/store multiple w/ just the pc
      */
     ldr     lr, =0f
     stmfd   sp!, {lr}
     ldmfd   sp!, {pc}
     nop

0:
     ldr     lr, =1f
     stmfa   sp!, {lr}
     ldmfa   sp!, {pc}
     nop

1:
     ldr     lr, =2f
     stmed   sp!, {lr}
     ldmed   sp!, {pc}
     nop

2:
     ldr     lr, =3f
     stmea   sp!, {lr}
     ldmea   sp!, {pc}
     nop

3:
     ldrnet  pc, [r0], IMM(4)
     mrsne   pc, cpsr
     mrsne   pc, spsr

     /*
      * Test pc to pc transfers
      */
     movne   pc, pc
     CLEAR_CARRY_BIT
     adcne   pc, pc, pc
     SET_CARRY_BIT
     adcne   pc, pc, pc
     addne   pc, pc, pc

     /*
      * Test Data Processing Immediate Instructions
      */
     ldrne   pc, =4
     CLEAR_CARRY_BIT
     adcne   pc, pc, IMM(4)
     SET_CARRY_BIT
     adcne   pc, pc, IMM(4)
     addne   pc, pc, IMM(4)

     ldr     r1, =0x12345678
     mvnne   pc, r1
     subne   pc, pc, r1
     sbcne   pc, pc, r1
     rsbne   pc, pc, r1
     rscne   pc, pc, r1
     andne   pc, pc, r1
     eorne   pc, pc, r1
     orrne   pc, pc, r1
     bicne   pc, pc, r1
     cmpne   pc, r1
     cmnne   pc, r1
     tstne   pc, r1
     teqne   pc, r1

     /*
      * Test Data Processing w/ Immediate Shift Instructions
      */
     ldr     r1, =0x12345678
     movne   pc, r1
     movne   pc, r1, lsl IMM(0)
     movne   pc, r1, lsr IMM(0)
     movne   pc, r1, asr IMM(0)
     movne   pc, r1, ror IMM(0)
     movne   pc, r1, lsl IMM(12)
     movne   pc, r1, lsr IMM(12)
     movne   pc, r1, asr IMM(12)
     movne   pc, r1, ror IMM(12)
     movne   pc, r1, lsr IMM(32)
     movne   pc, r1, asr IMM(32)
     CLEAR_CARRY_BIT
     movne   pc, r1, rrx
     SET_CARRY_BIT
     movne   pc, r1, rrx

     ldr     r1, =0x87654321
     movne   pc, r1
     movne   pc, r1, lsl IMM(0)
     movne   pc, r1, lsr IMM(0)
     movne   pc, r1, asr IMM(0)
     movne   pc, r1, ror IMM(0)
     movne   pc, r1, lsl IMM(12)
     movne   pc, r1, lsr IMM(12)
     movne   pc, r1, asr IMM(12)
     movne   pc, r1, ror IMM(12)
     movne   pc, r1, lsr IMM(32)
     movne   pc, r1, asr IMM(32)
     CLEAR_CARRY_BIT
     movne   pc, r1, rrx
     SET_CARRY_BIT
     movne   pc, r1, rrx

     /*
      * Test Branch and Branch/Link Instructions
      */
     bne     SYM_NAME(start)
     blne    SYM_NAME(start)

     /*
      * Test Branch Exchange Instructions
      */
     ldr     r1, =0x12345678
     bxne    r1

     /*
      * Test Data Processing w/ Register Shift Instructions
      */
     ldr     r1, =0x12345678
     ldr     r2, =0
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2
     ldr     r2, =12
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2
     ldr     r2, =32
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2
     ldr     r2, =45
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2

     ldr     r1, =0x87654321
     ldr     r2, =0
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2
     ldr     r2, =12
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2
     ldr     r2, =32
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2
     ldr     r2, =45
     movne   pc, r1, lsl r2
     movne   pc, r1, lsr r2
     movne   pc, r1, asr r2
     movne   pc, r1, ror r2

     /*
      * Test byte size loads of the PC with an immediate offset
      */
     ldrneb  pc, =(0x12345678+0)
     ldrneb  pc, =(0x12345678+1)
     ldrneb  pc, =(0x12345678+2)
     ldrneb  pc, =(0x12345678+3)
     ldrneb  pc, =(0x87654321+0)
     ldrneb  pc, =(0x87654321+1)
     ldrneb  pc, =(0x87654321+2)
     ldrneb  pc, =(0x87654321+3)

     /*
      * Test word size loads of the PC with an immediate offset
      */
     ldrne   pc, =(0x12345678+0)
     ldrne   pc, =(0x12345678+1)
     ldrne   pc, =(0x12345678+2)
     ldrne   pc, =(0x12345678+3)
     ldrne   pc, =(0x87654321+0)
     ldrne   pc, =(0x87654321+1)
     ldrne   pc, =(0x87654321+2)
     ldrne   pc, =(0x87654321+3)

     /*
      * Test byte and word size loads of the PC with a register offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x87654321
     ldrneb  pc, [r0, r1]
     ldrneb  pc, [r0, -r1]
     ldrne   pc, [r0, r1]
     ldrne   pc, [r0, -r1]


     /*
      * Test word size loads of the PC with a scaled register offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x12345678

     ldrne   pc, [r0, +r1, lsl IMM(0)]
     ldrne   pc, [r0, +r1, lsr IMM(0)]
     ldrne   pc, [r0, +r1, asr IMM(0)]
     ldrne   pc, [r0, +r1, ror IMM(0)]
     ldrne   pc, [r0, +r1, lsl IMM(12)]
     ldrne   pc, [r0, +r1, lsr IMM(12)]
     ldrne   pc, [r0, +r1, asr IMM(12)]
     ldrne   pc, [r0, +r1, ror IMM(12)]
     ldrne   pc, [r0, +r1, lsr IMM(32)]
     ldrne   pc, [r0, +r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]
     SET_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]

     ldrne   pc, [r0, -r1, lsl IMM(0)]
     ldrne   pc, [r0, -r1, lsr IMM(0)]
     ldrne   pc, [r0, -r1, asr IMM(0)]
     ldrne   pc, [r0, -r1, ror IMM(0)]
     ldrne   pc, [r0, -r1, lsl IMM(12)]
     ldrne   pc, [r0, -r1, lsr IMM(12)]
     ldrne   pc, [r0, -r1, asr IMM(12)]
     ldrne   pc, [r0, -r1, ror IMM(12)]
     ldrne   pc, [r0, -r1, lsr IMM(32)]
     ldrne   pc, [r0, -r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]
     SET_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]

     ldr     r1, =0x87654321

     ldrne   pc, [r0, +r1, lsl IMM(0)]
     ldrne   pc, [r0, +r1, lsr IMM(0)]
     ldrne   pc, [r0, +r1, asr IMM(0)]
     ldrne   pc, [r0, +r1, ror IMM(0)]
     ldrne   pc, [r0, +r1, lsl IMM(12)]
     ldrne   pc, [r0, +r1, lsr IMM(12)]
     ldrne   pc, [r0, +r1, asr IMM(12)]
     ldrne   pc, [r0, +r1, ror IMM(12)]
     ldrne   pc, [r0, +r1, lsr IMM(32)]
     ldrne   pc, [r0, +r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]
     SET_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]

     ldrne   pc, [r0, -r1, lsl IMM(0)]
     ldrne   pc, [r0, -r1, lsr IMM(0)]
     ldrne   pc, [r0, -r1, asr IMM(0)]
     ldrne   pc, [r0, -r1, ror IMM(0)]
     ldrne   pc, [r0, -r1, lsl IMM(12)]
     ldrne   pc, [r0, -r1, lsr IMM(12)]
     ldrne   pc, [r0, -r1, asr IMM(12)]
     ldrne   pc, [r0, -r1, ror IMM(12)]
     ldrne   pc, [r0, -r1, lsr IMM(32)]
     ldrne   pc, [r0, -r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]
     SET_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]

     /*
      * Test byte size loads of the PC with a scaled register offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x12345678

     ldrneb  pc, [r0, +r1, lsl IMM(0)]
     ldrneb  pc, [r0, +r1, lsr IMM(0)]
     ldrneb  pc, [r0, +r1, asr IMM(0)]
     ldrneb  pc, [r0, +r1, ror IMM(0)]
     ldrneb  pc, [r0, +r1, lsl IMM(12)]
     ldrneb  pc, [r0, +r1, lsr IMM(12)]
     ldrneb  pc, [r0, +r1, asr IMM(12)]
     ldrneb  pc, [r0, +r1, ror IMM(12)]
     ldrneb  pc, [r0, +r1, lsr IMM(32)]
     ldrneb  pc, [r0, +r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]
     SET_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]

     ldrneb  pc, [r0, -r1, lsl IMM(0)]
     ldrneb  pc, [r0, -r1, lsr IMM(0)]
     ldrneb  pc, [r0, -r1, asr IMM(0)]
     ldrneb  pc, [r0, -r1, ror IMM(0)]
     ldrneb  pc, [r0, -r1, lsl IMM(12)]
     ldrneb  pc, [r0, -r1, lsr IMM(12)]
     ldrneb  pc, [r0, -r1, asr IMM(12)]
     ldrneb  pc, [r0, -r1, ror IMM(12)]
     ldrneb  pc, [r0, -r1, lsr IMM(32)]
     ldrneb  pc, [r0, -r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]
     SET_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]

     ldr     r1, =0x87654321

     ldrneb  pc, [r0, +r1, lsl IMM(0)]
     ldrneb  pc, [r0, +r1, lsr IMM(0)]
     ldrneb  pc, [r0, +r1, asr IMM(0)]
     ldrneb  pc, [r0, +r1, ror IMM(0)]
     ldrneb  pc, [r0, +r1, lsl IMM(12)]
     ldrneb  pc, [r0, +r1, lsr IMM(12)]
     ldrneb  pc, [r0, +r1, asr IMM(12)]
     ldrneb  pc, [r0, +r1, ror IMM(12)]
     ldrneb  pc, [r0, +r1, lsr IMM(32)]
     ldrneb  pc, [r0, +r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]
     SET_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]

     ldrneb  pc, [r0, -r1, lsl IMM(0)]
     ldrneb  pc, [r0, -r1, lsr IMM(0)]
     ldrneb  pc, [r0, -r1, asr IMM(0)]
     ldrneb  pc, [r0, -r1, ror IMM(0)]
     ldrneb  pc, [r0, -r1, lsl IMM(12)]
     ldrneb  pc, [r0, -r1, lsr IMM(12)]
     ldrneb  pc, [r0, -r1, asr IMM(12)]
     ldrneb  pc, [r0, -r1, ror IMM(12)]
     ldrneb  pc, [r0, -r1, lsr IMM(32)]
     ldrneb  pc, [r0, -r1, asr IMM(32)]
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]
     SET_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]

     /*
      * Test byte and word size loads of the PC with 
      * an immediate preindexed offset
      */    
     ldr     r0, =0x12345678
     ldrne   pc, [r0,  IMM(4)]!
     ldrne   pc, [r0, IMM(-4)]!
     ldrneb  pc, [r0,  IMM(4)]!
     ldrneb  pc, [r0, IMM(-4)]!

     /*
      * Test byte and word size loads of the PC with 
      * an register preindexed offset
      */    
     ldr     r0, =0x12345678
     ldr     r1, =0x87654321
     ldrne   pc, [r0,  r1]!
     ldrne   pc, [r0, -r1]!
     ldrneb  pc, [r0,  r1]!
     ldrneb  pc, [r0, -r1]!

     /*
      * Test word size loads of the PC with a scaled register 
      * preindexed offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x12345678

     ldrne   pc, [r0, +r1, lsl IMM(0)]!
     ldrne   pc, [r0, +r1, lsr IMM(0)]!
     ldrne   pc, [r0, +r1, asr IMM(0)]!
     ldrne   pc, [r0, +r1, ror IMM(0)]!
     ldrne   pc, [r0, +r1, lsl IMM(12)]!
     ldrne   pc, [r0, +r1, lsr IMM(12)]!
     ldrne   pc, [r0, +r1, asr IMM(12)]!
     ldrne   pc, [r0, +r1, ror IMM(12)]!
     ldrne   pc, [r0, +r1, lsr IMM(32)]!
     ldrne   pc, [r0, +r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]!
     SET_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]!

     ldrne   pc, [r0, -r1, lsl IMM(0)]!
     ldrne   pc, [r0, -r1, lsr IMM(0)]!
     ldrne   pc, [r0, -r1, asr IMM(0)]!
     ldrne   pc, [r0, -r1, ror IMM(0)]!
     ldrne   pc, [r0, -r1, lsl IMM(12)]!
     ldrne   pc, [r0, -r1, lsr IMM(12)]!
     ldrne   pc, [r0, -r1, asr IMM(12)]!
     ldrne   pc, [r0, -r1, ror IMM(12)]!
     ldrne   pc, [r0, -r1, lsr IMM(32)]!
     ldrne   pc, [r0, -r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]!
     SET_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]!

     ldr     r1, =0x87654321

     ldrne   pc, [r0, +r1, lsl IMM(0)]!
     ldrne   pc, [r0, +r1, lsr IMM(0)]!
     ldrne   pc, [r0, +r1, asr IMM(0)]!
     ldrne   pc, [r0, +r1, ror IMM(0)]!
     ldrne   pc, [r0, +r1, lsl IMM(12)]!
     ldrne   pc, [r0, +r1, lsr IMM(12)]!
     ldrne   pc, [r0, +r1, asr IMM(12)]!
     ldrne   pc, [r0, +r1, ror IMM(12)]!
     ldrne   pc, [r0, +r1, lsr IMM(32)]!
     ldrne   pc, [r0, +r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]!
     SET_CARRY_BIT
     ldrne   pc, [r0, +r1, rrx]!

     ldrne   pc, [r0, -r1, lsl IMM(0)]!
     ldrne   pc, [r0, -r1, lsr IMM(0)]!
     ldrne   pc, [r0, -r1, asr IMM(0)]!
     ldrne   pc, [r0, -r1, ror IMM(0)]!
     ldrne   pc, [r0, -r1, lsl IMM(12)]!
     ldrne   pc, [r0, -r1, lsr IMM(12)]!
     ldrne   pc, [r0, -r1, asr IMM(12)]!
     ldrne   pc, [r0, -r1, ror IMM(12)]!
     ldrne   pc, [r0, -r1, lsr IMM(32)]!
     ldrne   pc, [r0, -r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]!
     SET_CARRY_BIT
     ldrne   pc, [r0, -r1, rrx]!

     /*
      * Test byte size loads of the PC with a scaled register 
      * preindexed offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x12345678

     ldrneb  pc, [r0, +r1, lsl IMM(0)]!
     ldrneb  pc, [r0, +r1, lsr IMM(0)]!
     ldrneb  pc, [r0, +r1, asr IMM(0)]!
     ldrneb  pc, [r0, +r1, ror IMM(0)]!
     ldrneb  pc, [r0, +r1, lsl IMM(12)]!
     ldrneb  pc, [r0, +r1, lsr IMM(12)]!
     ldrneb  pc, [r0, +r1, asr IMM(12)]!
     ldrneb  pc, [r0, +r1, ror IMM(12)]!
     ldrneb  pc, [r0, +r1, lsr IMM(32)]!
     ldrneb  pc, [r0, +r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]!
     SET_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]!

     ldrneb  pc, [r0, -r1, lsl IMM(0)]!
     ldrneb  pc, [r0, -r1, lsr IMM(0)]!
     ldrneb  pc, [r0, -r1, asr IMM(0)]!
     ldrneb  pc, [r0, -r1, ror IMM(0)]!
     ldrneb  pc, [r0, -r1, lsl IMM(12)]!
     ldrneb  pc, [r0, -r1, lsr IMM(12)]!
     ldrneb  pc, [r0, -r1, asr IMM(12)]!
     ldrneb  pc, [r0, -r1, ror IMM(12)]!
     ldrneb  pc, [r0, -r1, lsr IMM(32)]!
     ldrneb  pc, [r0, -r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]!
     SET_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]!

     ldr     r1, =0x87654321
     ldrneb  pc, [r0, +r1, lsl IMM(0)]!
     ldrneb  pc, [r0, +r1, lsr IMM(0)]!
     ldrneb  pc, [r0, +r1, asr IMM(0)]!
     ldrneb  pc, [r0, +r1, ror IMM(0)]!
     ldrneb  pc, [r0, +r1, lsl IMM(12)]!
     ldrneb  pc, [r0, +r1, lsr IMM(12)]!
     ldrneb  pc, [r0, +r1, asr IMM(12)]!
     ldrneb  pc, [r0, +r1, ror IMM(12)]!
     ldrneb  pc, [r0, +r1, lsr IMM(32)]!
     ldrneb  pc, [r0, +r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]!
     SET_CARRY_BIT
     ldrneb  pc, [r0, +r1, rrx]!

     ldrneb  pc, [r0, -r1, lsl IMM(0)]!
     ldrneb  pc, [r0, -r1, lsr IMM(0)]!
     ldrneb  pc, [r0, -r1, asr IMM(0)]!
     ldrneb  pc, [r0, -r1, ror IMM(0)]!
     ldrneb  pc, [r0, -r1, lsl IMM(12)]!
     ldrneb  pc, [r0, -r1, lsr IMM(12)]!
     ldrneb  pc, [r0, -r1, asr IMM(12)]!
     ldrneb  pc, [r0, -r1, ror IMM(12)]!
     ldrneb  pc, [r0, -r1, lsr IMM(32)]!
     ldrneb  pc, [r0, -r1, asr IMM(32)]!
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]!
     SET_CARRY_BIT
     ldrneb  pc, [r0, -r1, rrx]!

     /*
      * Test byte and word size loads of the PC with an immediate
      * postindexed offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x87654321
     ldrne   pc, [r0],  IMM(4)
     ldrne   pc, [r0], IMM(-4)
     ldrneb  pc, [r0],  IMM(4)
     ldrneb  pc, [r0], IMM(-4)

     /*
      * Test byte and word size loads of the PC with an register
      * postindexed offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x87654321
     ldrne   pc, [r0],  r1
     ldrne   pc, [r0], -r1
     ldrneb  pc, [r0],  r1
     ldrneb  pc, [r0], -r1

     /*
      * Test word size loads of the PC with a scaled register 
      * posindexed offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x12345678

     ldrne   pc, [r0], +r1, lsl IMM(0)
     ldrne   pc, [r0], +r1, lsr IMM(0)
     ldrne   pc, [r0], +r1, asr IMM(0)
     ldrne   pc, [r0], +r1, ror IMM(0)
     ldrne   pc, [r0], +r1, lsl IMM(12)
     ldrne   pc, [r0], +r1, lsr IMM(12)
     ldrne   pc, [r0], +r1, asr IMM(12)
     ldrne   pc, [r0], +r1, ror IMM(12)
     ldrne   pc, [r0], +r1, lsr IMM(32)
     ldrne   pc, [r0], +r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrne   pc, [r0], +r1, rrx
     SET_CARRY_BIT
     ldrne   pc, [r0], +r1, rrx

     ldrne   pc, [r0], -r1, lsl IMM(0)
     ldrne   pc, [r0], -r1, lsr IMM(0)
     ldrne   pc, [r0], -r1, asr IMM(0)
     ldrne   pc, [r0], -r1, ror IMM(0)
     ldrne   pc, [r0], -r1, lsl IMM(12)
     ldrne   pc, [r0], -r1, lsr IMM(12)
     ldrne   pc, [r0], -r1, asr IMM(12)
     ldrne   pc, [r0], -r1, ror IMM(12)
     ldrne   pc, [r0], -r1, lsr IMM(32)
     ldrne   pc, [r0], -r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrne   pc, [r0], -r1, rrx
     SET_CARRY_BIT
     ldrne   pc, [r0], -r1, rrx

     ldr     r1, =0x87654321

     ldrne   pc, [r0], +r1, lsl IMM(0)
     ldrne   pc, [r0], +r1, lsr IMM(0)
     ldrne   pc, [r0], +r1, asr IMM(0)
     ldrne   pc, [r0], +r1, ror IMM(0)
     ldrne   pc, [r0], +r1, lsl IMM(12)
     ldrne   pc, [r0], +r1, lsr IMM(12)
     ldrne   pc, [r0], +r1, asr IMM(12)
     ldrne   pc, [r0], +r1, ror IMM(12)
     ldrne   pc, [r0], +r1, lsr IMM(32)
     ldrne   pc, [r0], +r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrne   pc, [r0], +r1, rrx
     SET_CARRY_BIT
     ldrne   pc, [r0], +r1, rrx

     ldrne   pc, [r0], -r1, lsl IMM(0)
     ldrne   pc, [r0], -r1, lsr IMM(0)
     ldrne   pc, [r0], -r1, asr IMM(0)
     ldrne   pc, [r0], -r1, ror IMM(0)
     ldrne   pc, [r0], -r1, lsl IMM(12)
     ldrne   pc, [r0], -r1, lsr IMM(12)
     ldrne   pc, [r0], -r1, asr IMM(12)
     ldrne   pc, [r0], -r1, ror IMM(12)
     ldrne   pc, [r0], -r1, lsr IMM(32)
     ldrne   pc, [r0], -r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrne   pc, [r0], -r1, rrx
     SET_CARRY_BIT
     ldrne   pc, [r0], -r1, rrx

     /*
      * Test byte size loads of the PC with a scaled register 
      * preindexed offset
      */
     ldr     r0, =0x12345678
     ldr     r1, =0x12345678

     ldrneb  pc, [r0], +r1, lsl IMM(0)
     ldrneb  pc, [r0], +r1, lsr IMM(0)
     ldrneb  pc, [r0], +r1, asr IMM(0)
     ldrneb  pc, [r0], +r1, ror IMM(0)
     ldrneb  pc, [r0], +r1, lsl IMM(12)
     ldrneb  pc, [r0], +r1, lsr IMM(12)
     ldrneb  pc, [r0], +r1, asr IMM(12)
     ldrneb  pc, [r0], +r1, ror IMM(12)
     ldrneb  pc, [r0], +r1, lsr IMM(32)
     ldrneb  pc, [r0], +r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0], +r1, rrx
     SET_CARRY_BIT
     ldrneb  pc, [r0], +r1, rrx

     ldrneb  pc, [r0], -r1, lsl IMM(0)
     ldrneb  pc, [r0], -r1, lsr IMM(0)
     ldrneb  pc, [r0], -r1, asr IMM(0)
     ldrneb  pc, [r0], -r1, ror IMM(0)
     ldrneb  pc, [r0], -r1, lsl IMM(12)
     ldrneb  pc, [r0], -r1, lsr IMM(12)
     ldrneb  pc, [r0], -r1, asr IMM(12)
     ldrneb  pc, [r0], -r1, ror IMM(12)
     ldrneb  pc, [r0], -r1, lsr IMM(32)
     ldrneb  pc, [r0], -r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0], -r1, rrx
     SET_CARRY_BIT
     ldrneb  pc, [r0], -r1, rrx

     ldr     r1, =0x87654321
     ldrneb  pc, [r0], +r1, lsl IMM(0)
     ldrneb  pc, [r0], +r1, lsr IMM(0)
     ldrneb  pc, [r0], +r1, asr IMM(0)
     ldrneb  pc, [r0], +r1, ror IMM(0)
     ldrneb  pc, [r0], +r1, lsl IMM(12)
     ldrneb  pc, [r0], +r1, lsr IMM(12)
     ldrneb  pc, [r0], +r1, asr IMM(12)
     ldrneb  pc, [r0], +r1, ror IMM(12)
     ldrneb  pc, [r0], +r1, lsr IMM(32)
     ldrneb  pc, [r0], +r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0], +r1, rrx
     SET_CARRY_BIT
     ldrneb  pc, [r0], +r1, rrx

     ldrneb  pc, [r0], -r1, lsl IMM(0)
     ldrneb  pc, [r0], -r1, lsr IMM(0)
     ldrneb  pc, [r0], -r1, asr IMM(0)
     ldrneb  pc, [r0], -r1, ror IMM(0)
     ldrneb  pc, [r0], -r1, lsl IMM(12)
     ldrneb  pc, [r0], -r1, lsr IMM(12)
     ldrneb  pc, [r0], -r1, asr IMM(12)
     ldrneb  pc, [r0], -r1, ror IMM(12)
     ldrneb  pc, [r0], -r1, lsr IMM(32)
     ldrneb  pc, [r0], -r1, asr IMM(32)
     CLEAR_CARRY_BIT
     ldrneb  pc, [r0], -r1, rrx
     SET_CARRY_BIT
     ldrneb  pc, [r0], -r1, rrx
#endif /* TEST_SINGLESTEP */

     ldr     fp, =0			/* Null frame pointer */
     ldr     r7, =0			/* Null frame pointer for Thumb */
     b       SYM_NAME(c_start)
FUNC_END _start

#if TEST_SINGLESTEP

/*

Output from TEST_SINGLESTEP with DEBUG_SINGLESTEP enabled in singlestep.c
should be:

Cygnus BSP
CPU: ARM7TDMI
Board: ARM(R) Pid
Total RAM: 16777216 bytes
Top of RAM: 0x1000000
Setting BP at <0x00008004>: inst <0xe1a01000>
Setting BP at <0x00008008>: inst <0xe1500001>
Setting BP at <0x0000800c>: inst <0xe59fe03c>
Setting BP at <0x00008010>: inst <0xe92d5fff>
Setting BP at <0x00008014>: inst <0xe8bd9fff>
Setting BP at <0x00008018>: inst <0xe1a00000>
Setting BP at <0x0000801c>: inst <0xe59fe030>
Setting BP at <0x00008020>: inst <0xe9ad5fff>
Setting BP at <0x00008024>: inst <0xe83d9fff>
Setting BP at <0x00008028>: inst <0xe1a00000>
Setting BP at <0x0000802c>: inst <0xe59fe024>
Setting BP at <0x00008030>: inst <0xe82d5fff>
Setting BP at <0x00008034>: inst <0xe9bd9fff>
Setting BP at <0x00008038>: inst <0xe1a00000>
Setting BP at <0x0000803c>: inst <0xe59fe018>
Setting BP at <0x00008040>: inst <0xe8ad5fff>
Setting BP at <0x00008044>: inst <0xe93d9fff>
Setting BP at <0x00008048>: inst <0xe1a00000>
Setting BP at <0x0000804c>: inst <0xea000003>
Setting BP at <0x00008050>: inst <0x0000801c>
Setting BP at <0x00008060>: inst <0xe59fe03c>
Setting BP at <0x00008064>: inst <0xe92d5011>
Setting BP at <0x00008068>: inst <0xe8bd9011>
Setting BP at <0x0000806c>: inst <0xe1a00000>
Setting BP at <0x00008070>: inst <0xe59fe030>
Setting BP at <0x00008074>: inst <0xe9ad5011>
Setting BP at <0x00008078>: inst <0xe83d9011>
Setting BP at <0x0000807c>: inst <0xe1a00000>
Setting BP at <0x00008080>: inst <0xe59fe024>
Setting BP at <0x00008084>: inst <0xe82d5011>
Setting BP at <0x00008088>: inst <0xe9bd9011>
Setting BP at <0x0000808c>: inst <0xe1a00000>
Setting BP at <0x00008090>: inst <0xe59fe018>
Setting BP at <0x00008094>: inst <0xe8ad5011>
Setting BP at <0x00008098>: inst <0xe93d9011>
Setting BP at <0x0000809c>: inst <0xe1a00000>
Setting BP at <0x000080a0>: inst <0xea000003>
Setting BP at <0x000080a4>: inst <0x00008070>
Setting BP at <0x000080b4>: inst <0xe59fe03c>
Setting BP at <0x000080b8>: inst <0xe92d4000>
Setting BP at <0x000080bc>: inst <0xe8bd8000>
Setting BP at <0x000080c0>: inst <0xe1a00000>
Setting BP at <0x000080c4>: inst <0xe59fe030>
Setting BP at <0x000080c8>: inst <0xe9ad4000>
Setting BP at <0x000080cc>: inst <0xe83d8000>
Setting BP at <0x000080d0>: inst <0xe1a00000>
Setting BP at <0x000080d4>: inst <0xe59fe024>
Setting BP at <0x000080d8>: inst <0xe82d4000>
Setting BP at <0x000080dc>: inst <0xe9bd8000>
Setting BP at <0x000080e0>: inst <0xe1a00000>
Setting BP at <0x000080e4>: inst <0xe59fe018>
Setting BP at <0x000080e8>: inst <0xe8ad4000>
Setting BP at <0x000080ec>: inst <0xe93d8000>
Setting BP at <0x000080f0>: inst <0xe1a00000>
Setting BP at <0x000080f4>: inst <0xea000003>
Setting BP at <0x000080f8>: inst <0x000080c4>
Setting BP at <0x00008108>: inst <0x14b0f004>
Setting BP at <0x0000810c>: inst <0x110ff000>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008110>: inst <0x114ff000>
Setting BP at <0x6000001c>: Error
Setting BP at <0x00008114>: inst <0x11a0f00f>
Setting BP at <0x6000001c>: Error
Setting BP at <0x00008118>: inst <0xe10f2000>
Setting BP at <0x0000811c>: inst <0xe3c22202>
Setting BP at <0x0000811c>: inst <0xe3c22202>
Setting BP at <0x00008120>: inst <0xe129f002>
Setting BP at <0x00008124>: inst <0x10aff00f>
Setting BP at <0x00008128>: inst <0xe10f2000>
Setting BP at <0x00010258>: inst <0x00000000>
Setting BP at <0x0000812c>: inst <0xe3822202>
Setting BP at <0x00008130>: inst <0xe129f002>
Setting BP at <0x00008134>: inst <0x10aff00f>
Setting BP at <0x00008138>: inst <0x108ff00f>
Setting BP at <0x00010278>: inst <0x00000000>
Setting BP at <0x0000813c>: inst <0x13a0f004>
Setting BP at <0x00010280>: inst <0x00000000>
Setting BP at <0x00008140>: inst <0xe10f2000>
Setting BP at <0x00000004>: inst <0xe59ff014>
Setting BP at <0x00008144>: inst <0xe3c22202>
Setting BP at <0x00008148>: inst <0xe129f002>
Setting BP at <0x0000814c>: inst <0x12aff004>
Setting BP at <0x00008150>: inst <0xe10f2000>
Setting BP at <0x00008158>: inst <0xe129f002>
Setting BP at <0x00008154>: inst <0xe3822202>
Setting BP at <0x00008158>: inst <0xe129f002>
Setting BP at <0x0000815c>: inst <0x12aff004>
Setting BP at <0x00008160>: inst <0x128ff004>
Setting BP at <0x00008168>: inst <0x11e0f001>
Setting BP at <0x00008164>: inst <0xe59f1948>
Setting BP at <0x0000816c>: inst <0x104ff001>
Setting BP at <0x00008168>: inst <0x11e0f001>
Setting BP at <0x0000816c>: inst <0x104ff001>
Setting BP at <0xedcba984>: Error
Setting BP at <0x00008170>: inst <0x10cff001>
Setting BP at <0xedcc2afc>: Error
Setting BP at <0x00008174>: inst <0x106ff001>
Setting BP at <0xedcc2b00>: Error
Setting BP at <0x00008178>: inst <0x10eff001>
Setting BP at <0x1233d4fc>: Error
Setting BP at <0x0000817c>: inst <0x100ff001>
Setting BP at <0x1233d4f8>: Error
Setting BP at <0x00008180>: inst <0x102ff001>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008184>: inst <0x118ff001>
Setting BP at <0x1234d7f0>: Error
Setting BP at <0x00008188>: inst <0x11cff001>
Setting BP at <0x1234d7fc>: Error
Setting BP at <0x0000818c>: inst <0x115f0001>
Setting BP at <0x00008180>: inst <0x102ff001>
Setting BP at <0x00008190>: inst <0x117f0001>
Setting BP at <0x00008194>: inst <0x111f0001>
Setting BP at <0x00008198>: inst <0x113f0001>
Setting BP at <0x0000819c>: inst <0xe59f1910>
Setting BP at <0x000081a0>: inst <0x11a0f001>
Setting BP at <0x000081a4>: inst <0x11a0f001>
Setting BP at <0x12345678>: Error
Setting BP at <0x000081a8>: inst <0x11a0f001>
Setting BP at <0x12345678>: Error
Setting BP at <0x000081ac>: inst <0x11a0f001>
Setting BP at <0x12345678>: Error
Setting BP at <0x000081b0>: inst <0x11a0f001>
Setting BP at <0x12345678>: Error
Setting BP at <0x000081b4>: inst <0x11a0f601>
Setting BP at <0x12345678>: Error
Setting BP at <0x000081b8>: inst <0x11a0f621>
Setting BP at <0x45678000>: Error
Setting BP at <0x000081bc>: inst <0x11a0f641>
Setting BP at <0x00012344>: inst <0x11951515>
Setting BP at <0x000081c0>: inst <0x11a0f661>
Setting BP at <0x00012344>: inst <0x11951515>
Setting BP at <0x000081c4>: inst <0x11a0f021>
Setting BP at <0x67812344>: Error
Setting BP at <0x000081c8>: inst <0x11a0f041>
Setting BP at <0x00000000>: Error
Setting BP at <0x000081cc>: inst <0xe10f2000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000081d0>: inst <0xe3c22202>
Setting BP at <0x000081d4>: inst <0xe129f002>
Setting BP at <0x000081d8>: inst <0x11a0f061>
Setting BP at <0x000081dc>: inst <0xe10f2000>
Setting BP at <0x091a2b3c>: inst <0xe24cb004>
Setting BP at <0x000081e0>: inst <0xe3822202>
Setting BP at <0x000081e4>: inst <0xe129f002>
Setting BP at <0x000081e8>: inst <0x11a0f061>
Setting BP at <0x000081ec>: inst <0xe59f18c4>
Setting BP at <0x891a2b3c>: Error
Setting BP at <0x000081f0>: inst <0x11a0f001>
Setting BP at <0x000081f4>: inst <0x11a0f001>
Setting BP at <0x87654320>: Error
Setting BP at <0x000081f8>: inst <0x11a0f001>
Setting BP at <0x87654320>: Error
Setting BP at <0x000081fc>: inst <0x11a0f001>
Setting BP at <0x87654320>: Error
Setting BP at <0x00008200>: inst <0x11a0f001>
Setting BP at <0x87654320>: Error
Setting BP at <0x00008204>: inst <0x11a0f601>
Setting BP at <0x87654320>: Error
Setting BP at <0x00008208>: inst <0x11a0f621>
Setting BP at <0x54321000>: Error
Setting BP at <0x0000820c>: inst <0x11a0f641>
Setting BP at <0x00087654>: inst <0x04001f50>
Setting BP at <0x00008210>: inst <0x11a0f661>
Setting BP at <0xfff87654>: Error
Setting BP at <0x00008214>: inst <0x11a0f021>
Setting BP at <0x32187654>: Error
Setting BP at <0x00008218>: inst <0x11a0f041>
Setting BP at <0x00000000>: Error
Setting BP at <0x0000821c>: inst <0xe10f2000>
Setting BP at <0xfffffffc>: Error
Setting BP at <0x00008220>: inst <0xe3c22202>
Setting BP at <0x00008224>: inst <0xe129f002>
Setting BP at <0x00008228>: inst <0x11a0f061>
Setting BP at <0x0000822c>: inst <0xe10f2000>
Setting BP at <0x43b2a190>: Error
Setting BP at <0x00008230>: inst <0xe3822202>
Setting BP at <0x00008234>: inst <0xe129f002>
Setting BP at <0x00008238>: inst <0x11a0f061>
Setting BP at <0x0000823c>: inst <0x1affff6f>
Setting BP at <0xc3b2a190>: Error
Setting BP at <0x00008240>: inst <0x1bffff6e>
Setting BP at <0x00008000>: inst <0xe3a00000>
Setting BP at <0x00008244>: inst <0xe59f1868>
Setting BP at <0x00008000>: inst <0xe3a00000>
Setting BP at <0x00008248>: inst <0x112fff11>
Setting BP at <0x0000824c>: inst <0xe59f1860>
Setting BP at <0x12345678>: Error
Setting BP at <0x00008250>: inst <0xe3a02000>
Setting BP at <0x00008254>: inst <0x11a0f211>
Setting BP at <0x00008258>: inst <0x11a0f231>
Setting BP at <0x12345678>: Error
Setting BP at <0x0000825c>: inst <0x11a0f251>
Setting BP at <0x12345678>: Error
Setting BP at <0x00008260>: inst <0x11a0f271>
Setting BP at <0x12345678>: Error
Setting BP at <0x00008264>: inst <0xe3a0200c>
Setting BP at <0x12345678>: Error
Setting BP at <0x00008268>: inst <0x11a0f211>
Setting BP at <0x0000826c>: inst <0x11a0f231>
Setting BP at <0x45678000>: Error
Setting BP at <0x00008270>: inst <0x11a0f251>
Setting BP at <0x00012344>: inst <0x11951515>
Setting BP at <0x00008274>: inst <0x11a0f271>
Setting BP at <0x00012344>: inst <0x11951515>
Setting BP at <0x00008278>: inst <0xe3a02020>
Setting BP at <0x67812344>: Error
Setting BP at <0x0000827c>: inst <0x11a0f211>
Setting BP at <0x00008280>: inst <0x11a0f231>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008284>: inst <0x11a0f251>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008288>: inst <0x11a0f271>
Setting BP at <0x00000000>: Error
Setting BP at <0x0000828c>: inst <0xe3a0202d>
Setting BP at <0x12345678>: Error
Setting BP at <0x00008290>: inst <0x11a0f211>
Setting BP at <0x00008294>: inst <0x11a0f231>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008298>: inst <0x11a0f251>
Setting BP at <0x00000000>: Error
Setting BP at <0x0000829c>: inst <0x11a0f271>
Setting BP at <0x00000000>: Error
Setting BP at <0x000082a0>: inst <0xe59f1810>
Setting BP at <0xb3c091a0>: Error
Setting BP at <0x000082a4>: inst <0xe3a02000>
Setting BP at <0x000082a8>: inst <0x11a0f211>
Setting BP at <0x000082ac>: inst <0x11a0f231>
Setting BP at <0x87654320>: Error
Setting BP at <0x000082b0>: inst <0x11a0f251>
Setting BP at <0x87654320>: Error
Setting BP at <0x000082b4>: inst <0x11a0f271>
Setting BP at <0x87654320>: Error
Setting BP at <0x000082b8>: inst <0xe3a0200c>
Setting BP at <0x87654320>: Error
Setting BP at <0x000082bc>: inst <0x11a0f211>
Setting BP at <0x000082c0>: inst <0x11a0f231>
Setting BP at <0x54321000>: Error
Setting BP at <0x000082c4>: inst <0x11a0f251>
Setting BP at <0x00087654>: inst <0x04001f50>
Setting BP at <0x000082c8>: inst <0x11a0f271>
Setting BP at <0xfff87654>: Error
Setting BP at <0x000082cc>: inst <0xe3a02020>
Setting BP at <0x32187654>: Error
Setting BP at <0x000082d0>: inst <0x11a0f211>
Setting BP at <0x000082d4>: inst <0x11a0f231>
Setting BP at <0x00000000>: Error
Setting BP at <0x000082d8>: inst <0x11a0f251>
Setting BP at <0x00000000>: Error
Setting BP at <0x000082dc>: inst <0x11a0f271>
Setting BP at <0xfffffffc>: Error
Setting BP at <0x000082e0>: inst <0xe3a0202d>
Setting BP at <0x87654320>: Error
Setting BP at <0x000082e4>: inst <0x11a0f211>
Setting BP at <0x000082e8>: inst <0x11a0f231>
Setting BP at <0x00000000>: Error
Setting BP at <0x000082ec>: inst <0x11a0f251>
Setting BP at <0x00000000>: Error
Setting BP at <0x000082f0>: inst <0x11a0f271>
Setting BP at <0xfffffffc>: Error
Setting BP at <0x000082f4>: inst <0x15dff7b8>
Setting BP at <0x190c3b28>: Error
Setting BP at <0x000082f8>: inst <0x15dff7b5>
Setting BP at *(0x00008ab4): data <0x00000078>
Setting BP at <0x00000078>: inst <0x00000000>
Setting BP at <0x000082fc>: inst <0x15dff7b2>
Setting BP at *(0x00008ab4): data <0x00000056>
Setting BP at <0x00000056>: Error
Setting BP at <0x00008300>: inst <0x15dff7af>
Setting BP at *(0x00008ab4): data <0x00000034>
Setting BP at <0x00000034>: inst <0x04001d80>
Setting BP at <0x00008304>: inst <0x15dff7ac>
Setting BP at *(0x00008ab4): data <0x00000012>
Setting BP at <0x00000012>: Error
Setting BP at <0x00008308>: inst <0x15dff7a9>
Setting BP at *(0x00008ab8): data <0x00000021>
Setting BP at <0x00000021>: Error
Setting BP at <0x0000830c>: inst <0x15dff7a6>
Setting BP at *(0x00008ab8): data <0x00000043>
Setting BP at <0x00000043>: Error
Setting BP at <0x00008310>: inst <0x15dff7a3>
Setting BP at *(0x00008ab8): data <0x00000065>
Setting BP at <0x00000065>: Error
Setting BP at <0x00008314>: inst <0x159ff798>
Setting BP at *(0x00008ab8): data <0x00000087>
Setting BP at <0x00000087>: Error
Setting BP at <0x00008318>: inst <0x159ff795>
Setting BP at *(0x00008ab4): data <0x12345678>
Setting BP at <0x12345678>: Error
Setting BP at <0x0000831c>: inst <0x159ff792>
Setting BP at *(0x00008ab4): data <0x12345678>
Setting BP at <0x78123456>: Error
Setting BP at <0x00008320>: inst <0x159ff78f>
Setting BP at *(0x00008ab4): data <0x12345678>
Setting BP at <0x56781234>: Error
Setting BP at <0x00008324>: inst <0x159ff78c>
Setting BP at *(0x00008ab4): data <0x12345678>
Setting BP at <0x34567812>: Error
Setting BP at <0x00008328>: inst <0x159ff789>
Setting BP at *(0x00008ab8): data <0x87654321>
Setting BP at <0x87654321>: Error
Setting BP at <0x0000832c>: inst <0x159ff786>
Setting BP at *(0x00008ab8): data <0x87654321>
Setting BP at <0x21876543>: Error
Setting BP at <0x00008330>: inst <0x159ff783>
Setting BP at *(0x00008ab8): data <0x87654321>
Setting BP at <0x43218765>: Error
Setting BP at <0x00008334>: inst <0xe59f0778>
Setting BP at *(0x00008ab8): data <0x87654321>
Setting BP at <0x65432187>: Error
Setting BP at <0x00008338>: inst <0xe59f1778>
Setting BP at <0x0000833c>: inst <0x17d0f001>
Setting BP at <0x00008340>: inst <0x1750f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x00008344>: inst <0x1790f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008348>: inst <0x1710f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x0000834c>: inst <0xe59f0760>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008350>: inst <0xe59f175c>
Setting BP at <0x00008354>: inst <0x1790f001>
Setting BP at <0x00008358>: inst <0x1790f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x0000835c>: inst <0x1790f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008360>: inst <0x1790f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008364>: inst <0x1790f601>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008368>: inst <0x1790f621>
Setting BP at *(0x579bd678): Error
Setting BP at <0x0000836c>: inst <0x1790f641>
Setting BP at *(0x123579bc): Error
Setting BP at <0x00008370>: inst <0x1790f661>
Setting BP at *(0x123579bc): Error
Setting BP at <0x00008374>: inst <0x1790f021>
Setting BP at *(0x79b579bc): Error
Setting BP at <0x00008378>: inst <0x1790f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000837c>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008380>: inst <0xe3c22202>
Setting BP at <0x00008384>: inst <0xe129f002>
Setting BP at <0x00008388>: inst <0x1790f061>
Setting BP at <0x0000838c>: inst <0xe10f2000>
Setting BP at *(0x1b4e81b4): Error
Setting BP at <0x00008390>: inst <0xe3822202>
Setting BP at <0x00008394>: inst <0xe129f002>
Setting BP at <0x00008398>: inst <0x1790f061>
Setting BP at <0x0000839c>: inst <0x1710f001>
Setting BP at *(0x9b4e81b4): Error
Setting BP at <0x000083a0>: inst <0x1710f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000083a4>: inst <0x1710f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000083a8>: inst <0x1710f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000083ac>: inst <0x1710f601>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000083b0>: inst <0x1710f621>
Setting BP at *(0xccccd678): Error
Setting BP at <0x000083b4>: inst <0x1710f641>
Setting BP at *(0x12333330): Error
Setting BP at <0x000083b8>: inst <0x1710f661>
Setting BP at *(0x12333330): Error
Setting BP at <0x000083bc>: inst <0x1710f021>
Setting BP at *(0xaab33330): Error
Setting BP at <0x000083c0>: inst <0x1710f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000083c4>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000083c8>: inst <0xe3c22202>
Setting BP at <0x000083cc>: inst <0xe129f002>
Setting BP at <0x000083d0>: inst <0x1710f061>
Setting BP at <0x000083d4>: inst <0xe10f2000>
Setting BP at *(0x091a2b3c): data <0xe24cb004>
Setting BP at <0xe24cb004>: Error
Setting BP at <0x000083d8>: inst <0xe3822202>
Setting BP at <0x000083dc>: inst <0xe129f002>
Setting BP at <0x000083e0>: inst <0x1710f061>
Setting BP at <0x000083e4>: inst <0xe59f16cc>
Setting BP at *(0x891a2b3c): Error
Setting BP at <0x000083e8>: inst <0x1790f001>
Setting BP at <0x000083ec>: inst <0x1790f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000083f0>: inst <0x1790f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000083f4>: inst <0x1790f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000083f8>: inst <0x1790f601>
Setting BP at *(0x99999998): Error
Setting BP at <0x000083fc>: inst <0x1790f621>
Setting BP at *(0x66666678): Error
Setting BP at <0x00008400>: inst <0x1790f641>
Setting BP at *(0x123ccccc): Error
Setting BP at <0x00008404>: inst <0x1790f661>
Setting BP at *(0x122ccccc): Error
Setting BP at <0x00008408>: inst <0x1790f021>
Setting BP at *(0x444ccccc): Error
Setting BP at <0x0000840c>: inst <0x1790f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008410>: inst <0xe10f2000>
Setting BP at *(0x12345674): Error
Setting BP at <0x00008414>: inst <0xe3c22202>
Setting BP at <0x00008418>: inst <0xe129f002>
Setting BP at <0x0000841c>: inst <0x1790f061>
Setting BP at <0x00008420>: inst <0xe10f2000>
Setting BP at *(0x55e6f808): Error
Setting BP at <0x00008424>: inst <0xe3822202>
Setting BP at <0x00008428>: inst <0xe129f002>
Setting BP at <0x0000842c>: inst <0x1790f061>
Setting BP at <0x00008430>: inst <0x1710f001>
Setting BP at *(0xd5e6f808): Error
Setting BP at <0x00008434>: inst <0x1710f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008438>: inst <0x1710f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x0000843c>: inst <0x1710f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008440>: inst <0x1710f601>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008444>: inst <0x1710f621>
Setting BP at *(0xbe024678): Error
Setting BP at <0x00008448>: inst <0x1710f641>
Setting BP at *(0x122be024): Error
Setting BP at <0x0000844c>: inst <0x1710f661>
Setting BP at *(0x123be024): Error
Setting BP at <0x00008450>: inst <0x1710f021>
Setting BP at *(0xe01be024): Error
Setting BP at <0x00008454>: inst <0x1710f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008458>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000845c>: inst <0xe3c22202>
Setting BP at <0x00008460>: inst <0xe129f002>
Setting BP at <0x00008464>: inst <0x1710f061>
Setting BP at <0x00008468>: inst <0xe10f2000>
Setting BP at *(0xce81b4e8): Error
Setting BP at <0x0000846c>: inst <0xe3822202>
Setting BP at <0x00008470>: inst <0xe129f002>
Setting BP at <0x00008474>: inst <0x1710f061>
Setting BP at <0x00008478>: inst <0xe59f0634>
Setting BP at *(0x4e81b4e8): Error
Setting BP at <0x0000847c>: inst <0xe59f1630>
Setting BP at <0x00008480>: inst <0x17d0f001>
Setting BP at <0x00008484>: inst <0x17d0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008488>: inst <0x17d0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x0000848c>: inst <0x17d0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008490>: inst <0x17d0f601>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008494>: inst <0x17d0f621>
Setting BP at *(0x579bd678): Error
Setting BP at <0x00008498>: inst <0x17d0f641>
Setting BP at *(0x123579bc): Error
Setting BP at <0x0000849c>: inst <0x17d0f661>
Setting BP at *(0x123579bc): Error
Setting BP at <0x000084a0>: inst <0x17d0f021>
Setting BP at *(0x79b579bc): Error
Setting BP at <0x000084a4>: inst <0x17d0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000084a8>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000084ac>: inst <0xe3c22202>
Setting BP at <0x000084b0>: inst <0xe129f002>
Setting BP at <0x000084b4>: inst <0x17d0f061>
Setting BP at <0x000084b8>: inst <0xe10f2000>
Setting BP at *(0x1b4e81b4): Error
Setting BP at <0x000084bc>: inst <0xe3822202>
Setting BP at <0x000084c0>: inst <0xe129f002>
Setting BP at <0x000084c4>: inst <0x17d0f061>
Setting BP at <0x000084c8>: inst <0x1750f001>
Setting BP at *(0x9b4e81b4): Error
Setting BP at <0x000084cc>: inst <0x1750f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000084d0>: inst <0x1750f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000084d4>: inst <0x1750f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000084d8>: inst <0x1750f601>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x000084dc>: inst <0x1750f621>
Setting BP at *(0xccccd678): Error
Setting BP at <0x000084e0>: inst <0x1750f641>
Setting BP at *(0x12333330): Error
Setting BP at <0x000084e4>: inst <0x1750f661>
Setting BP at *(0x12333330): Error
Setting BP at <0x000084e8>: inst <0x1750f021>
Setting BP at *(0xaab33330): Error
Setting BP at <0x000084ec>: inst <0x1750f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000084f0>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000084f4>: inst <0xe3c22202>
Setting BP at <0x000084f8>: inst <0xe129f002>
Setting BP at <0x000084fc>: inst <0x1750f061>
Setting BP at <0x00008500>: inst <0xe10f2000>
Setting BP at *(0x091a2b3c): data <0x00000004>
Setting BP at <0x00000004>: inst <0xe59ff014>
Setting BP at <0x00008504>: inst <0xe3822202>
Setting BP at <0x00008508>: inst <0xe129f002>
Setting BP at <0x0000850c>: inst <0x1750f061>
Setting BP at <0x00008510>: inst <0xe59f15a0>
Setting BP at *(0x891a2b3c): Error
Setting BP at <0x00008514>: inst <0x17d0f001>
Setting BP at <0x00008518>: inst <0x17d0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x0000851c>: inst <0x17d0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x00008520>: inst <0x17d0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x00008524>: inst <0x17d0f601>
Setting BP at *(0x99999998): Error
Setting BP at <0x00008528>: inst <0x17d0f621>
Setting BP at *(0x66666678): Error
Setting BP at <0x0000852c>: inst <0x17d0f641>
Setting BP at *(0x123ccccc): Error
Setting BP at <0x00008530>: inst <0x17d0f661>
Setting BP at *(0x122ccccc): Error
Setting BP at <0x00008534>: inst <0x17d0f021>
Setting BP at *(0x444ccccc): Error
Setting BP at <0x00008538>: inst <0x17d0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000853c>: inst <0xe10f2000>
Setting BP at *(0x12345674): Error
Setting BP at <0x00008540>: inst <0xe3c22202>
Setting BP at <0x00008544>: inst <0xe129f002>
Setting BP at <0x00008548>: inst <0x17d0f061>
Setting BP at <0x0000854c>: inst <0xe10f2000>
Setting BP at *(0x55e6f808): Error
Setting BP at <0x00008550>: inst <0xe3822202>
Setting BP at <0x00008554>: inst <0xe129f002>
Setting BP at <0x00008558>: inst <0x17d0f061>
Setting BP at <0x0000855c>: inst <0x1750f001>
Setting BP at *(0xd5e6f808): Error
Setting BP at <0x00008560>: inst <0x1750f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008564>: inst <0x1750f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008568>: inst <0x1750f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x0000856c>: inst <0x1750f601>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x00008570>: inst <0x1750f621>
Setting BP at *(0xbe024678): Error
Setting BP at <0x00008574>: inst <0x1750f641>
Setting BP at *(0x122be024): Error
Setting BP at <0x00008578>: inst <0x1750f661>
Setting BP at *(0x123be024): Error
Setting BP at <0x0000857c>: inst <0x1750f021>
Setting BP at *(0xe01be024): Error
Setting BP at <0x00008580>: inst <0x1750f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008584>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008588>: inst <0xe3c22202>
Setting BP at <0x0000858c>: inst <0xe129f002>
Setting BP at <0x00008590>: inst <0x1750f061>
Setting BP at <0x00008594>: inst <0xe10f2000>
Setting BP at *(0xce81b4e8): Error
Setting BP at <0x00008598>: inst <0xe3822202>
Setting BP at <0x0000859c>: inst <0xe129f002>
Setting BP at <0x000085a0>: inst <0x1750f061>
Setting BP at <0x000085a4>: inst <0xe59f0508>
Setting BP at *(0x4e81b4e8): Error
Setting BP at <0x000085a8>: inst <0x15b0f004>
Setting BP at <0x000085ac>: inst <0x1530f004>
Setting BP at *(0x1234567c): Error
Setting BP at <0x000085b0>: inst <0x15f0f004>
Setting BP at *(0x12345674): Error
Setting BP at <0x000085b4>: inst <0x1570f004>
Setting BP at *(0x1234567c): Error
Setting BP at <0x000085b8>: inst <0xe59f04f4>
Setting BP at *(0x12345674): Error
Setting BP at <0x000085bc>: inst <0xe59f14f4>
Setting BP at <0x000085c0>: inst <0x17b0f001>
Setting BP at <0x000085c4>: inst <0x1730f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000085c8>: inst <0x17f0f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000085cc>: inst <0x1770f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000085d0>: inst <0xe59f04dc>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000085d4>: inst <0xe59f14d8>
Setting BP at <0x000085d8>: inst <0x17b0f001>
Setting BP at <0x000085dc>: inst <0x17b0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x000085e0>: inst <0x17b0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x000085e4>: inst <0x17b0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x000085e8>: inst <0x17b0f601>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x000085ec>: inst <0x17b0f621>
Setting BP at *(0x579bd678): Error
Setting BP at <0x000085f0>: inst <0x17b0f641>
Setting BP at *(0x123579bc): Error
Setting BP at <0x000085f4>: inst <0x17b0f661>
Setting BP at *(0x123579bc): Error
Setting BP at <0x000085f8>: inst <0x17b0f021>
Setting BP at *(0x79b579bc): Error
Setting BP at <0x000085fc>: inst <0x17b0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008600>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008604>: inst <0xe3c22202>
Setting BP at <0x00008608>: inst <0xe129f002>
Setting BP at <0x0000860c>: inst <0x17b0f061>
Setting BP at <0x00008610>: inst <0xe10f2000>
Setting BP at *(0x1b4e81b4): Error
Setting BP at <0x00008614>: inst <0xe3822202>
Setting BP at <0x00008618>: inst <0xe129f002>
Setting BP at <0x0000861c>: inst <0x17b0f061>
Setting BP at <0x00008620>: inst <0x1730f001>
Setting BP at *(0x9b4e81b4): Error
Setting BP at <0x00008624>: inst <0x1730f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008628>: inst <0x1730f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x0000862c>: inst <0x1730f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008630>: inst <0x1730f601>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008634>: inst <0x1730f621>
Setting BP at *(0xccccd678): Error
Setting BP at <0x00008638>: inst <0x1730f641>
Setting BP at *(0x12333330): Error
Setting BP at <0x0000863c>: inst <0x1730f661>
Setting BP at *(0x12333330): Error
Setting BP at <0x00008640>: inst <0x1730f021>
Setting BP at *(0xaab33330): Error
Setting BP at <0x00008644>: inst <0x1730f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008648>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000864c>: inst <0xe3c22202>
Setting BP at <0x00008650>: inst <0xe129f002>
Setting BP at <0x00008654>: inst <0x1730f061>
Setting BP at <0x00008658>: inst <0xe10f2000>
Setting BP at *(0x091a2b3c): data <0xe24cb004>
Setting BP at <0xe24cb004>: Error
Setting BP at <0x0000865c>: inst <0xe3822202>
Setting BP at <0x00008660>: inst <0xe129f002>
Setting BP at <0x00008664>: inst <0x1730f061>
Setting BP at <0x00008668>: inst <0xe59f1448>
Setting BP at *(0x891a2b3c): Error
Setting BP at <0x0000866c>: inst <0x17b0f001>
Setting BP at <0x00008670>: inst <0x17b0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x00008674>: inst <0x17b0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x00008678>: inst <0x17b0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x0000867c>: inst <0x17b0f601>
Setting BP at *(0x99999998): Error
Setting BP at <0x00008680>: inst <0x17b0f621>
Setting BP at *(0x66666678): Error
Setting BP at <0x00008684>: inst <0x17b0f641>
Setting BP at *(0x123ccccc): Error
Setting BP at <0x00008688>: inst <0x17b0f661>
Setting BP at *(0x122ccccc): Error
Setting BP at <0x0000868c>: inst <0x17b0f021>
Setting BP at *(0x444ccccc): Error
Setting BP at <0x00008690>: inst <0x17b0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008694>: inst <0xe10f2000>
Setting BP at *(0x12345674): Error
Setting BP at <0x00008698>: inst <0xe3c22202>
Setting BP at <0x0000869c>: inst <0xe129f002>
Setting BP at <0x000086a0>: inst <0x17b0f061>
Setting BP at <0x000086a4>: inst <0xe10f2000>
Setting BP at *(0x55e6f808): Error
Setting BP at <0x000086a8>: inst <0xe3822202>
Setting BP at <0x000086ac>: inst <0xe129f002>
Setting BP at <0x000086b0>: inst <0x17b0f061>
Setting BP at <0x000086b4>: inst <0x1730f001>
Setting BP at *(0xd5e6f808): Error
Setting BP at <0x000086b8>: inst <0x1730f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000086bc>: inst <0x1730f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000086c0>: inst <0x1730f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000086c4>: inst <0x1730f601>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000086c8>: inst <0x1730f621>
Setting BP at *(0xbe024678): Error
Setting BP at <0x000086cc>: inst <0x1730f641>
Setting BP at *(0x122be024): Error
Setting BP at <0x000086d0>: inst <0x1730f661>
Setting BP at *(0x123be024): Error
Setting BP at <0x000086d4>: inst <0x1730f021>
Setting BP at *(0xe01be024): Error
Setting BP at <0x000086d8>: inst <0x1730f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000086dc>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000086e0>: inst <0xe3c22202>
Setting BP at <0x000086e4>: inst <0xe129f002>
Setting BP at <0x000086e8>: inst <0x1730f061>
Setting BP at <0x000086ec>: inst <0xe10f2000>
Setting BP at *(0xce81b4e8): Error
Setting BP at <0x000086f0>: inst <0xe3822202>
Setting BP at <0x000086f4>: inst <0xe129f002>
Setting BP at <0x000086f8>: inst <0x1730f061>
Setting BP at <0x000086fc>: inst <0xe59f03b0>
Setting BP at *(0x4e81b4e8): Error
Setting BP at <0x00008700>: inst <0xe59f13ac>
Setting BP at <0x00008704>: inst <0x17f0f001>
Setting BP at <0x00008708>: inst <0x17f0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x0000870c>: inst <0x17f0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008710>: inst <0x17f0f001>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008714>: inst <0x17f0f601>
Setting BP at *(0x2468acf0): Error
Setting BP at <0x00008718>: inst <0x17f0f621>
Setting BP at *(0x579bd678): Error
Setting BP at <0x0000871c>: inst <0x17f0f641>
Setting BP at *(0x123579bc): Error
Setting BP at <0x00008720>: inst <0x17f0f661>
Setting BP at *(0x123579bc): Error
Setting BP at <0x00008724>: inst <0x17f0f021>
Setting BP at *(0x79b579bc): Error
Setting BP at <0x00008728>: inst <0x17f0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000872c>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008730>: inst <0xe3c22202>
Setting BP at <0x00008734>: inst <0xe129f002>
Setting BP at <0x00008738>: inst <0x17f0f061>
Setting BP at <0x0000873c>: inst <0xe10f2000>
Setting BP at *(0x1b4e81b4): Error
Setting BP at <0x00008740>: inst <0xe3822202>
Setting BP at <0x00008744>: inst <0xe129f002>
Setting BP at <0x00008748>: inst <0x17f0f061>
Setting BP at <0x0000874c>: inst <0x1770f001>
Setting BP at *(0x9b4e81b4): Error
Setting BP at <0x00008750>: inst <0x1770f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008754>: inst <0x1770f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008758>: inst <0x1770f001>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x0000875c>: inst <0x1770f601>
Setting BP at *(0x00000000): data <0x00000000>
Setting BP at <0x00000000>: Error
Setting BP at <0x00008760>: inst <0x1770f621>
Setting BP at *(0xccccd678): Error
Setting BP at <0x00008764>: inst <0x1770f641>
Setting BP at *(0x12333330): Error
Setting BP at <0x00008768>: inst <0x1770f661>
Setting BP at *(0x12333330): Error
Setting BP at <0x0000876c>: inst <0x1770f021>
Setting BP at *(0xaab33330): Error
Setting BP at <0x00008770>: inst <0x1770f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008774>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008778>: inst <0xe3c22202>
Setting BP at <0x0000877c>: inst <0xe129f002>
Setting BP at <0x00008780>: inst <0x1770f061>
Setting BP at <0x00008784>: inst <0xe10f2000>
Setting BP at *(0x091a2b3c): data <0x00000004>
Setting BP at <0x00000004>: inst <0xe59ff014>
Setting BP at <0x00008788>: inst <0xe3822202>
Setting BP at <0x0000878c>: inst <0xe129f002>
Setting BP at <0x00008790>: inst <0x1770f061>
Setting BP at <0x00008794>: inst <0xe59f131c>
Setting BP at *(0x891a2b3c): Error
Setting BP at <0x00008798>: inst <0x17f0f001>
Setting BP at <0x0000879c>: inst <0x17f0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000087a0>: inst <0x17f0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000087a4>: inst <0x17f0f001>
Setting BP at *(0x99999998): Error
Setting BP at <0x000087a8>: inst <0x17f0f601>
Setting BP at *(0x99999998): Error
Setting BP at <0x000087ac>: inst <0x17f0f621>
Setting BP at *(0x66666678): Error
Setting BP at <0x000087b0>: inst <0x17f0f641>
Setting BP at *(0x123ccccc): Error
Setting BP at <0x000087b4>: inst <0x17f0f661>
Setting BP at *(0x122ccccc): Error
Setting BP at <0x000087b8>: inst <0x17f0f021>
Setting BP at *(0x444ccccc): Error
Setting BP at <0x000087bc>: inst <0x17f0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000087c0>: inst <0xe10f2000>
Setting BP at *(0x12345674): Error
Setting BP at <0x000087c4>: inst <0xe3c22202>
Setting BP at <0x000087c8>: inst <0xe129f002>
Setting BP at <0x000087cc>: inst <0x17f0f061>
Setting BP at <0x000087d0>: inst <0xe10f2000>
Setting BP at *(0x55e6f808): Error
Setting BP at <0x000087d4>: inst <0xe3822202>
Setting BP at <0x000087d8>: inst <0xe129f002>
Setting BP at <0x000087dc>: inst <0x17f0f061>
Setting BP at <0x000087e0>: inst <0x1770f001>
Setting BP at *(0xd5e6f808): Error
Setting BP at <0x000087e4>: inst <0x1770f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000087e8>: inst <0x1770f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000087ec>: inst <0x1770f001>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000087f0>: inst <0x1770f601>
Setting BP at *(0x8acf1354): Error
Setting BP at <0x000087f4>: inst <0x1770f621>
Setting BP at *(0xbe024678): Error
Setting BP at <0x000087f8>: inst <0x1770f641>
Setting BP at *(0x122be024): Error
Setting BP at <0x000087fc>: inst <0x1770f661>
Setting BP at *(0x123be024): Error
Setting BP at <0x00008800>: inst <0x1770f021>
Setting BP at *(0xe01be024): Error
Setting BP at <0x00008804>: inst <0x1770f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008808>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000880c>: inst <0xe3c22202>
Setting BP at <0x00008810>: inst <0xe129f002>
Setting BP at <0x00008814>: inst <0x1770f061>
Setting BP at <0x00008818>: inst <0xe10f2000>
Setting BP at *(0xce81b4e8): Error
Setting BP at <0x0000881c>: inst <0xe3822202>
Setting BP at <0x00008820>: inst <0xe129f002>
Setting BP at <0x00008824>: inst <0x1770f061>
Setting BP at <0x00008828>: inst <0xe59f0284>
Setting BP at *(0x4e81b4e8): Error
Setting BP at <0x0000882c>: inst <0xe59f1284>
Setting BP at <0x00008830>: inst <0x1490f004>
Setting BP at <0x00008834>: inst <0x1410f004>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008838>: inst <0x14d0f004>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000883c>: inst <0x1450f004>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008840>: inst <0xe59f026c>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008844>: inst <0xe59f126c>
Setting BP at <0x00008848>: inst <0x1690f001>
Setting BP at <0x0000884c>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008850>: inst <0x16d0f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008854>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008858>: inst <0xe59f0254>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000885c>: inst <0xe59f1250>
Setting BP at <0x00008860>: inst <0x1690f001>
Setting BP at <0x00008864>: inst <0x1690f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008868>: inst <0x1690f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000886c>: inst <0x1690f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008870>: inst <0x1690f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008874>: inst <0x1690f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008878>: inst <0x1690f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000887c>: inst <0x1690f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008880>: inst <0x1690f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008884>: inst <0x1690f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008888>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000888c>: inst <0xe3c22202>
Setting BP at <0x00008890>: inst <0xe129f002>
Setting BP at <0x00008894>: inst <0x1690f061>
Setting BP at <0x00008898>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000889c>: inst <0xe3822202>
Setting BP at <0x000088a0>: inst <0xe129f002>
Setting BP at <0x000088a4>: inst <0x1690f061>
Setting BP at <0x000088a8>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088ac>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088b0>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088b4>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088b8>: inst <0x1610f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088bc>: inst <0x1610f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088c0>: inst <0x1610f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088c4>: inst <0x1610f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088c8>: inst <0x1610f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088cc>: inst <0x1610f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088d0>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088d4>: inst <0xe3c22202>
Setting BP at <0x000088d8>: inst <0xe129f002>
Setting BP at <0x000088dc>: inst <0x1610f061>
Setting BP at <0x000088e0>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088e4>: inst <0xe3822202>
Setting BP at <0x000088e8>: inst <0xe129f002>
Setting BP at <0x000088ec>: inst <0x1610f061>
Setting BP at <0x000088f0>: inst <0xe59f11c0>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088f4>: inst <0x1690f001>
Setting BP at <0x000088f8>: inst <0x1690f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000088fc>: inst <0x1690f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008900>: inst <0x1690f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008904>: inst <0x1690f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008908>: inst <0x1690f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000890c>: inst <0x1690f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008910>: inst <0x1690f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008914>: inst <0x1690f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008918>: inst <0x1690f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000891c>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008920>: inst <0xe3c22202>
Setting BP at <0x00008924>: inst <0xe129f002>
Setting BP at <0x00008928>: inst <0x1690f061>
Setting BP at <0x0000892c>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008930>: inst <0xe3822202>
Setting BP at <0x00008934>: inst <0xe129f002>
Setting BP at <0x00008938>: inst <0x1690f061>
Setting BP at <0x0000893c>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008940>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008944>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008948>: inst <0x1610f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000894c>: inst <0x1610f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008950>: inst <0x1610f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008954>: inst <0x1610f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008958>: inst <0x1610f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000895c>: inst <0x1610f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008960>: inst <0x1610f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008964>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008968>: inst <0xe3c22202>
Setting BP at <0x0000896c>: inst <0xe129f002>
Setting BP at <0x00008970>: inst <0x1610f061>
Setting BP at <0x00008974>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008978>: inst <0xe3822202>
Setting BP at <0x0000897c>: inst <0xe129f002>
Setting BP at <0x00008980>: inst <0x1610f061>
Setting BP at <0x00008984>: inst <0xe59f0128>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008988>: inst <0xe59f1124>
Setting BP at <0x0000898c>: inst <0x16d0f001>
Setting BP at <0x00008990>: inst <0x16d0f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008994>: inst <0x16d0f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008998>: inst <0x16d0f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x0000899c>: inst <0x16d0f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089a0>: inst <0x16d0f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089a4>: inst <0x16d0f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089a8>: inst <0x16d0f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089ac>: inst <0x16d0f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089b0>: inst <0x16d0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089b4>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089b8>: inst <0xe3c22202>
Setting BP at <0x000089bc>: inst <0xe129f002>
Setting BP at <0x000089c0>: inst <0x16d0f061>
Setting BP at <0x000089c4>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089c8>: inst <0xe3822202>
Setting BP at <0x000089cc>: inst <0xe129f002>
Setting BP at <0x000089d0>: inst <0x16d0f061>
Setting BP at <0x000089d4>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089d8>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089dc>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089e0>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089e4>: inst <0x1650f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089e8>: inst <0x1650f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089ec>: inst <0x1650f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089f0>: inst <0x1650f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089f4>: inst <0x1650f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089f8>: inst <0x1650f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x000089fc>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a00>: inst <0xe3c22202>
Setting BP at <0x00008a04>: inst <0xe129f002>
Setting BP at <0x00008a08>: inst <0x1650f061>
Setting BP at <0x00008a0c>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a10>: inst <0xe3822202>
Setting BP at <0x00008a14>: inst <0xe129f002>
Setting BP at <0x00008a18>: inst <0x1650f061>
Setting BP at <0x00008a1c>: inst <0xe59f1094>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a20>: inst <0x16d0f001>
Setting BP at <0x00008a24>: inst <0x16d0f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a28>: inst <0x16d0f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a2c>: inst <0x16d0f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a30>: inst <0x16d0f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a34>: inst <0x16d0f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a38>: inst <0x16d0f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a3c>: inst <0x16d0f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a40>: inst <0x16d0f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a44>: inst <0x16d0f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a48>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a4c>: inst <0xe3c22202>
Setting BP at <0x00008a50>: inst <0xe129f002>
Setting BP at <0x00008a54>: inst <0x16d0f061>
Setting BP at <0x00008a58>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a5c>: inst <0xe3822202>
Setting BP at <0x00008a60>: inst <0xe129f002>
Setting BP at <0x00008a64>: inst <0x16d0f061>
Setting BP at <0x00008a68>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a6c>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a70>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a74>: inst <0x1650f001>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a78>: inst <0x1650f601>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a7c>: inst <0x1650f621>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a80>: inst <0x1650f641>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a84>: inst <0x1650f661>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a88>: inst <0x1650f021>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a8c>: inst <0x1650f041>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a90>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008a94>: inst <0xe3c22202>
Setting BP at <0x00008a98>: inst <0xe129f002>
Setting BP at <0x00008a9c>: inst <0x1650f061>
Setting BP at <0x00008aa0>: inst <0xe10f2000>
Setting BP at *(0x12345678): Error
Setting BP at <0x00008aa4>: inst <0xe3822202>
Setting BP at <0x00008aa8>: inst <0xe129f002>
Setting BP at <0x00008aac>: inst <0x1650f061>
Setting BP at <0x00008ab0>: inst <0xea000001>
Setting BP at *(0x12345678): Error

*/

#endif /* TEST_SINGLESTEP */
