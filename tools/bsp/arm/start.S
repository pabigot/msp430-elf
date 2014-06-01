/*
 * start.S -- ROM-based startup for ARM(R)
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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
        .file   "start.S"
	
	/*
	 * If USE_AIF_HEADER is defined, prepend an ARM(R) AIF header to the ROM image
	 * so that the Cygnus BSP can co-exist with ARM(R) bootloader. It will also be
	 * necessary to modify the link address as the bootloader will occupy the
	 * the default bsp link address.
	 */
#if defined(USE_AIF_HEADER)
	/* These two symbols are created in the linker script */
	.extern SYM_NAME(_ROSize)
	.extern SYM_NAME(_RWSize)
	
	.section ".aif_header","ax"
	nop
	nop
	nop
	.word 0
	nop
	.word SYM_NAME(_ROSize) - 0x80
	.word SYM_NAME(_RWSize)
	.word 0
	.word 0
	.word 0
	.word SYM_NAME(aif_hdr_end) + 0x40
	.p2align 7
SYM_NAME(aif_hdr_end):
#endif

        .text
FUNC_START start

#ifdef __YAVAPAI__
	b       real_start
	ldr     pc, SYM_NAME(_UNDEFINED_ISR)
	ldr     pc, SYM_NAME(_SWI_ISR)
	ldr     pc, SYM_NAME(_PREFETCH_ABORT_ISR)
	ldr     pc, SYM_NAME(_DATA_ABORT_ISR)
	ldr     pc, SYM_NAME(_ADDR_ERROR_ISR)
	ldr     pc, SYM_NAME(_IRQ_ISR)
	ldr     pc, SYM_NAME(_FIQ_ISR)

SYM_NAME(_UNDEFINED_ISR):      .word   SYM_NAME(UNDEFINED_VECTOR)
SYM_NAME(_SWI_ISR):            .word   SYM_NAME(SWI_VECTOR)
SYM_NAME(_PREFETCH_ABORT_ISR): .word   SYM_NAME(PREFETCH_ABORT_VECTOR)
SYM_NAME(_DATA_ABORT_ISR):     .word   SYM_NAME(DATA_ABORT_VECTOR)
SYM_NAME(_ADDR_ERROR_ISR):     .word   SYM_NAME(ADDR_ERROR_VECTOR)
SYM_NAME(_IRQ_ISR):            .word   SYM_NAME(IRQ_VECTOR)
SYM_NAME(_FIQ_ISR):            .word   SYM_NAME(FIQ_VECTOR)
#endif

real_start:
	/*
	 * This nop seems necessary when using a PROMJet ROM emulator
	 * on SA-110 boards. It does no harm, so leave it in
	 * for all boards.
	 */
	nop

        /*
         * Careful.  There is no memory yet.
         */
	
        /*
         * Setup the initial exception stack
         */
	ldr	r0,=(ARM_PSR_IRQ | ARM_PSR_FIQ | ARM_PSR_MODE_SVC)
	msr	cpsr, r0
	ldr	sp, SYM_NAME(_ex_stack_addr)

        /*
         * We manually put the return address into the lr in case
         * _bsp_init_stack modifies the memory layout and invalidates
         * the current map
         */
        ldr     lr, SYM_NAME(_c_rom_start_addr)
        b       SYM_NAME(_bsp_init_stack)

SYM_NAME(_ex_stack_addr): .word SYM_NAME(_ex_stack)
SYM_NAME(_c_rom_start_addr): .word SYM_NAME(c_rom_start)

#ifdef __YAVAPAI__
	/* we have to skip over Yavapai internal regs */
	.p2align 13
	.global SYM_NAME(__REAL_TEXT_START)
SYM_NAME(__REAL_TEXT_START):
#endif

FUNC_END start


