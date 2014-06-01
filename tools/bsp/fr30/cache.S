/*
 * cache.S -- FR30 I-cache ops.
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
	.file "cache.S"

#include <bsp/cpu.h>

	.text	

#define ENAB	1
#define FLSH	2
#define ELKR	4
#define EOLK	8
#define ALFL   16
#define GBLK   32

FUNC_START __icache_init
	ldi:20	IMM(0x3e7),r4
	ldi:8	IMM(FLSH+ELKR),r5
	stb	r5,@r4
	ret
FUNC_END __icache_init

FUNC_START __icache_enable
	ldi:20	IMM(0x3e7),r4
	ldi:8	IMM(ENAB),r5
	stb	r5,@r4
	nop
	nop
	nop
	ret
FUNC_END __icache_enable

FUNC_START __icache_disable
	ldi:20	IMM(0x3e7),r4
	bandl	IMM(0xe),@r4
	nop
	nop
	nop
	ret
FUNC_END __icache_disable

FUNC_START __icache_flush
	ldi:20	IMM(0x3e7),r4
	borl	IMM(FLSH),@r4
	nop
	nop
	nop
	ret
FUNC_END __icache_flush
