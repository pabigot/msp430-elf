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
	.file "init_sim.S"
	
#include <bsp/cpu.h>
	
	.text

FUNC_START _bsp_init_stack
	seti	0x007bfff0,sp
	seti	8*1024*1024,gr4
	sti	gr4,@(sp,0)
	ret
FUNC_END _bsp_init_stack
