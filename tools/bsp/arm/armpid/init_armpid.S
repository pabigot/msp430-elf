/*
 * init_armpid.S -- Initialization for ARM(R) Pid Eval board.
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
 * Other Brands and Trademarks are the property of their respective owners.
 */
	.file "init_pid.S"
        .title "Init Code for ARM(R) Pid Eval Board"

#include <bsp/cpu.h>
#include <bsp/armpid.h>
        
#define DEBUG_INIT_STACK 0
	
	.text

        .align     4
FUNC_START _bsp_init_stack
        /*
         *
         * Load the PC directly with the address of the next instruction
         * so that we start executing from ROM.
         */
        ldr     pc, =0f
0:
        BOARD_BOOT_LED_DEBUG 1

        /*
         * Write to the CLEAR_RESET_MAP register and redo the memory
         * map.  RAM will now be located at 0.
         */
        ldr     r11, =ARMPID_REMAP_CLEAR_RST_MAP
        ldr     r10, =0
        strb    r10, [r11]

        BOARD_BOOT_LED_DEBUG 2

        /*
         * Setup a real stack pointer
         */
        ldr     sp, =(RAM_ACTUAL_BASE + RAM_TOTAL_SIZE)

        /*
         * Setup config_data parameter for call to c_start
         */
        mov     r0, sp

        /*
         * Return (effectively calling c_start)
         */
        mov     pc, lr
FUNC_END _bsp_init_stack
