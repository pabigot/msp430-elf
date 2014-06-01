/*
 * init_sim.S -- Initialization for simulator based stub.
 *
 * Copyright (c) 1999 Cygnus Support
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
	.file 1 "init_sim.S"
	
#include <bsp/cpu.h>
	
	.text
        .align     2
	.set noreorder
	.set noat

FUNC_START _bsp_init_stack
	

	li	s0,4*1024*1024
	lui	sp,0x8000
	or	sp,sp,s0
	addiu	sp,sp,-64
	
	/* store pointer to be copied to
	   _bsp_ram_info_ptr, later */
#ifdef __mips64
	sd	s0,0(sp)
#else
	sw	s0,0(sp)
#endif
	jr	ra
	 nop
FUNC_END _bsp_init_stack



