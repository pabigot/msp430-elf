/*
 * start.S -- FR30 BSP startup.
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

        .file   "start.S"

        .extern SYM_NAME(_bsp_init_stack)
        .extern SYM_NAME(_bsp_init)
        .extern SYM_NAME(_start)

        /*
         *  Reset Vector.
         *
         *  This vector is always in ROM (0xffffc).
         *  It is placed there by the linker script using a special section.
         */
        .section ".fr30_reset_vector","ax"
        .long   SYM_NAME(_start)

        /*
         *  Start of text segment.
         */
        .text
        .p2align 1

SYM_NAME(__vector_start):
        .rept 256
        st      rp,@-r15
        call    SYM_NAME(__exception)
        .endr
        
        /*
         * We get here in a supervisor stack frame like:
         *
         *     ssp+8 --> ps   // pre-eit program status word
         *     ssp+4 --> pc   // eit return address
         *     ssp   --> rp   // pre-eit rp register
         *
         * Because a call instruction was used to get here, the vector number
         * of the eit may be calculated from the rp register:
         *     vec_addr = rp - 4
         *     vec_num  = (0x3fc - (vec_addr - tbr))/4 = vec_num
         */
FUNC_START __exception
        st      r0,@-r15
        st      r14,@-r15
        ldi:8   IMM(20),r0
        add     r15,r0
        st      r0,@-r15
        /*
         *     ssp+20 --> ps   // pre-eit program status word
         *     ssp+16 --> pc   // eit return address
         *     ssp+12 --> rp   // pre-eit rp register
         *     ssp+8  --> r0
         *     ssp+4  --> r14
         *     ssp    --> pre-exception SSP
         */
        ldi:32  IMM(SYM_NAME(_ex_stack)),r0
        cmp     r0,r15
        ble     1f      /* branch if r15 <= _ex_stack */

        /* must switch stack */
        mov     r15,r14
        mov     r0,r15
        /*
         * Copy those registers already pushed.
         */
        ld      @(r14,20),r0
        st      r0,@-r15
        ld      @(r14,16),r0
        st      r0,@-r15
        ld      @(r14,12),r0
        st      r0,@-r15
        ld      @(r14,8),r0
        st      r0,@-r15
        ld      @(r14,4),r0
        st      r0,@-r15
        ld      @r14,r0
        st      r0,@-r15
    1:
        st      USP,@-r15
        st      TBR,@-r15
        st      MDL,@-r15
        st      MDH,@-r15
        stm1    (r8,r9,r10,r11,r12,r13)
        stm0    (r1,r2,r3,r4,r5,r6,r7)

        /*
         * Figure out which pre-exception stack we were using
         * and push it on the exception stack as r15.
         */
        addn2   IMM(-4),r15
        mov     r15,r14
        ldi:8   FRAME_PS,r0
        add     r15,r0
        btsth   IMM(2),@r0
        beq     1f
        ld      @(r14,FRAME_USP),r0
        bra     2f
    1:
        ld      @(r14,FRAME_SSP),r0
    2:
        st      r0,@r15
        
        /* calculate vector number */
        mov     rp,r4
        addn2   IMM(-4),r4  /* r0 = rp - 4 */
        ldi:32  IMM(SYM_NAME(__vector_start)),r0
        sub     r0,r4
        lsr     IMM(2),r4   /* r4 = ((rp-4) - __vector_start) / 4 = bsp_exc_num */
        
        /*
         * If its a breakpoint, adjust PC to point to it.
         */
        cmp     IMM(9),r4
        bne     1f
        ld      @(r14,FRAME_PC),r3
        addn2   IMM(-2),r3
        st      r3,@(r14,FRAME_PC)
    1:
        mov     SSP,r5
        st      r4,@-r15
        ldi:32  IMM(SYM_NAME(_bsp_exc_dispatch)),r0
        call    @r0
        or      r4,r4
        ld      @r15+,r4
        bne     1f

        mov     SSP,r5
        ldi:32  IMM(SYM_NAME(_bsp_dbg_vector)),r0
        ld      @r0,r0
        call    @r0
    1:
        add     IMM(4),r15
        ldm0    (r1,r2,r3,r4,r5,r6,r7)
        ldm1    (r8,r9,r10,r11,r12,r13)
        ld      @r15+,MDH
        ld      @r15+,MDL
        ld      @r15+,TBR
        ld      @r15+,USP
        ld      @r15+,r14
        /*
         * At this point, we have:
         *
         *     r14 = pre-exception SSP
         *
         *     ssp+16 --> ps   // pre-eit program status word
         *     ssp+12 --> pc   // eit return address
         *     ssp+8  --> rp   // pre-eit rp register
         *     ssp+4  --> r0
         *     ssp    --> r14
         */
        ldi:8   IMM(20),r0
        sub     r0,r14          /* adjust to post-exception SSP */
        cmp     r15,r14
        beq     1f
        /*
         * Need to swap stacks.
         */
        ld      @r15+,r0
        st      r0,@r14
        ld      @r15+,r0
        st      r0,@(r14,4)
        ld      @r15+,r0
        st      r0,@(r14,8)
        ld      @r15+,r0
        st      r0,@(r14,12)
        ld      @r15+,r0
        st      r0,@(r14,16)
        mov     r14,r15
    1:
        ld      @r15+,r14
        ld      @r15+,r0
        ld      @r15+,RP

                reti
FUNC_END __exception

/*
 * This is the HW reset entry point.
 */
FUNC_START _start
        /*
         * Perform low-level HW initialization and establish a stack.
         */
        ldi:32  IMM(SYM_NAME(_bsp_init_stack)),r0
        call    @r0
        
        /*
         *  Move vector table to RAM.
         */     
#if 0   
        ldi:32  IMM(SYM_NAME(__vector_start)) + 255*4,r0
        ldi:32  IMM(SYM_NAME(__table_base)),r4
        mov     r4,TBR
        ldi:20  IMM(255),r1
    1:
        st      r0,@r4
        addn2   IMM(-4),r0
        add2    IMM(-1),r1
        bne:d   1b
         add    IMM(4),r4
#else
        ldi:32  IMM(SYM_NAME(__table_base)),r4
        mov     r4,TBR
#endif

        ldi:32  IMM(SYM_NAME(c_rom_start)),r0
        call    @r0
FUNC_END _start

        /*
         * Initialized data
         */
        .data

        /* Hardware EIT vector table */
SYM_NAME(__table_base):
        .set    __offset,255*4
        .rept 255
        .long   SYM_NAME(__vector_start) + __offset
        .set    __offset,__offset-4
        .endr

        /*
         *  Provide a stack for use exceptions and interrupts.
         *  Note that this stack is in .bss, so it will be below
         *  the normal user stack which is placed in high memory.
         */
        .section ".bss","a"
        .p2align 4
        .rept   1024
        .long   0
        .endr
SYM_NAME(_ex_stack):


