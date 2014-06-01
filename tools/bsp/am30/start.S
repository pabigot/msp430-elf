/*
 * start.S -- AM30 (MN10300x) BSP ROM-based startup.
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
 */

#include <bsp/cpu.h>
#include "gdb.h"

        .file   "start.S"

        .text

        /* Reset vector */
        jmp     SYM_NAME(_start)

        /* NMI vector */
        .p2align 3
        jmp     SYM_NAME(_nmi_handler)

        /* Trap vector */
        .p2align 3
        jmp     SYM_NAME(_trap_handler)


FUNC_START _start
        /*
         *  Jump to _bsp_init_stack with return address in a0.
         *  We call _bsp_init_stack this way because there is
         *  no stack, yet.
         */
        mov     SYM_NAME(_bsp_init_stack),a1
        mov     SYM_NAME(c_rom_start),a0
        jmp     (a1)
FUNC_END _start


        /*
         * The following macros are used to build and teardown
         * an exception stack with a frame that looks like:
         *
         *      +------------------+
         * +56  |       pc         |
         *      +------------------+
         * +52  |       psw        |
         *      +------------------+
         * +48  |     pre-nmi sp   |
         *      +------------------+
         * +44  |       d2         |
         *      +------------------+
         * +40  |       d3         |
         *      +------------------+
         * +36  |       a2         |
         *      +------------------+
         * +32  |       a3         |
         *      +------------------+
         * +28  |       d0         |
         *      +------------------+
         * +24  |       d1         |
         *      +------------------+
         * +20  |       a0         |
         *      +------------------+
         * +16  |       a1         |
         *      +------------------+
         * +12  |       mdr        |
         *      +------------------+
         * +08  |       lir        |
         *      +------------------+
         * +04  |       lar        |
         *      +------------------+
         *  00  |      dummy       |
         *      +------------------+
         */
#define PC_OFFSET 56
#define DUMMY_OFFSET 0
        
        
.macro  BUILD_EXCEPTION_FRAME
        /*
         * Make sure we're using exception stack.
         */
        mov     a0,(__a0_save)                  # save a0
        mov     sp,a0                           # compare current sp to
        cmp     _ex_stack,a0                    # gdb stack
        ble     1f

        /*
         * Need to switch stacks.
         */
        mov     _ex_stack-8,a0
        mov     sp,(0,a0)                       # save current sp
        mov     a0,sp                           # use exception stack
        mov     (__a0_save),a0                  # restore a0
        movm    [d2,d3,a2,a3,other],(sp)        # push all registers

        /*
         * copy pc and psw to new stack.
         * IS THIS NECCESSARY???
         */
        mov     (48,sp),a0                      # original sp
        mov     (0,a0),d0
        mov     d0,(52,sp)
        mov     (4,a0),d0
        mov     d0,(56,sp)

        /*
         *  Adjust saved SP to pre-NMI value.
         */
        add     8,a0
        mov     a0,(48,sp)

        bra     2f
  1:
        add     -4,sp
        mov     (__a0_save),a0                  # restore a0
        movm    [d2,d3,a2,a3,other],(sp)        # push all registers
        clr     d0                              # store zero for 
        mov     d0,(48,sp)                      # saved sp
  2:
.endm


.macro  TEARDOWN_EXCEPTION_FRAME
        mov     (48,sp),a0
        cmp     0,a0
        bne     1f
        /*
         * Simple case. Keep same stack.
         */
        movm    (sp),[d2,d3,a2,a3,other]        # pop all registers
        add     4,sp                            # skip saved sp
        jmp     2f
    1:
        /*
         *  Copy PSW and PC to final stack.
         */
        mov     (48,sp),a0                      # new sp
        mov     (52,sp),d0                      # copy psw
        mov     d0,(-8,a0)
        mov     (56,sp),d0                      # copy pc
        mov     d0,(-4,a0)
        
        movm    (sp),[d2,d3,a2,a3,other]        # pop regs
        mov     a0,(__a0_save)
        mov     sp,a0
        mov     (0,a0),sp                       # restore stack pointer
        mov     (__a0_save),a0                  # restore a0
        add     -8,sp
    2:
.endm

        
        /*
         * Low-level interrupt handler. There are 7 distinct priority levels
         * for interrupts. Any interrupt can be assigned to any priority level.
         * Each priority level has its own handler vector which must reside in
         * the first 64K bytes of ROM. This BSP doesn't make any special use of
         * this priority scheme. That is, all priority levels are serviced by
         * this same vector.
         */
FUNC_START _interrupt_handler
        BUILD_EXCEPTION_FRAME

        /* store signal id in dummy field of frame */
        mov     TARGET_SIGNAL_INT,d0
        mov     d0,(DUMMY_OFFSET,sp)

        mov     sp,a0
        mov     a0,d0
        add     -4,sp                           # make space for return link
        calls   SYM_NAME(_cpu_irq_dispatch)
        add     4,sp

        cmp     0,d0
        bne     1f
        /*
         * zero return value means higher-level code
         * did not handle the interrupt. Call debug
         * stub.
         */
        mov     sp,a0
        mov     a0,d1
        mov     BSP_EXC_IRQ,d0
        mov     (SYM_NAME(_bsp_dbg_vector)),a0
        add     -4,sp
        calls   (a0)
        add     4,sp
    1:
        TEARDOWN_EXCEPTION_FRAME
        rti
FUNC_END _interrupt_handler


        /*
         * Low-level NMI handler. The NMI on the AM30 is heavily overloaded.
         * Possible causes for an NMI exception are:
         *   + Hardware NMI (surprise, surprise)
         *   + Watchdog timer overflow.
         *   + System errors such as unaligned access or illegal opcode.
         *
         * NMI exception frame looks like:
         *      +------------------+
         * SP+4 |       pc         |
         *      +------------------+
         * SP   |       psw        |
         *      +------------------+
         */
FUNC_START _nmi_handler
        BUILD_EXCEPTION_FRAME

        /* store signal id in dummy field of frame */
        mov     TARGET_SIGNAL_INT,d0
        mov     d0,(DUMMY_OFFSET,sp)

        mov     sp,a0           /* all of the handlers expect a single */
        mov     a0,d0           /* argument pointing to the exception frame */
        movhu   (ICRBASE),d1    /* interrupt control register */

        btst    1,d1
        bne     1f
        /* its a HW NMI */
        mov     1,d1
        movhu   d1,(ICRBASE)
        add     -4,sp
        calls   SYM_NAME(_handle_nmi_interrupt)
        add     4,sp
        bra     done
    1:
        btst    2,d1
        bne     1f
        /* its a watchdog interrupt */
        mov     2,d1
        movhu   d1,(ICRBASE)
        add     -4,sp
        calls   SYM_NAME(_handle_watchdog_interrupt)
        add     4,sp
        bra     done
                
        /* its a system error */
        mov     4,d1
        movhu   d1,(ICRBASE)
        add     -4,sp
        calls   SYM_NAME(_cpu_nmi_dispatch)
        add     4,sp

    done:
        cmp     0,d0
        bne     1f
        /*
         * zero return value means higher-level code
         * did not handle the NMI. Call debug stub.
         */
        mov     sp,a0
        mov     a0,d1
        mov     BSP_EXC_IRQ,d0
        mov     (SYM_NAME(_bsp_dbg_vector)),a0
        add     -4,sp
        calls   (a0)
        add     4,sp
    1:
        TEARDOWN_EXCEPTION_FRAME
        rti
FUNC_END _nmi_handler

        
        /*
         *  Low-level trap handler.
         */
FUNC_START _trap_handler
        add     -8,sp                           # adjust below dummy psw
        movm    [d2,d3,a2,a3,other],(sp)        # push all registers
        mov     sp,a0
        mov     a0,d1                           # pointer to trap frame
        add     56,a0                           # a0 = SP before trap
        mov     a0,(48,sp)                      # store original sp
        mov     BSP_EXC_TRAP,d0
        add     -4,sp
        calls   SYM_NAME(_bsp_exc_dispatch)
        add     4,sp
        movm    (sp),[d2,d3,a2,a3,other]        # pop all registers
        add     8,sp
        rets
FUNC_END _trap_handler

        /*
         *  Provide a stack for use by nmi exception handler.
         *  Note that this stack is in .bss, so it will be below
         *  the normal user stack which is placed in high memory.
         *  We use this fact to detect whether or not we need to
         *  switch stacks before saving regs.
         */
        .section ".bss","a"
        .p2align 4
        .rept   4096
        .long   0
        .endr
_ex_stack:      
        .long   0

__a0_save:
        .long 0
