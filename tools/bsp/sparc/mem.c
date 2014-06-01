/*
 * mem.c -- SPARC specific support for safe memory read/write.
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
#include <stdlib.h>
#include <setjmp.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"

/*
 * exports from mem-asm.S
 */
extern void __load_8a(void *src, void *dest);
extern unsigned int __load_8a_insn;
extern void __store_8a(void *src, void *dest);
extern unsigned int __store_8a_insn;
extern void __load_16a(void *src, void *dest);
extern unsigned int __load_16a_insn;
extern void __store_16a(void *src, void *dest);
extern unsigned int __store_16a_insn;
extern void __load_32a(void *src, void *dest);
extern unsigned int __load_32a_insn;
extern void __store_32a(void *src, void *dest);
extern unsigned int __store_32a_insn;


typedef void (*moveproc_t)(void *s, void *d);

static jmp_buf __errjmp;

/*
 * These are globals because we want them preserved
 * across function calls.
 */
static bsp_handler_t __oldtrap;
static int __done;


static void
move_8(void *src, void *dest)
{
    *(char *)dest = *(char *)src;
}


static void
move_16(void *src, void *dest)
{
    *(short *)dest = *(short *)src;
}


static void
move_32(void *src, void *dest)
{
    *(int *)dest = *(int *)src;
}


static int
err_trap(int exc_nr, void *regs)
{
    longjmp(__errjmp, 1);
}


int
bsp_memory_read(void *addr,    /* start addr of memory to read */
		int  asid,     /* address space id */
		int  rsize,    /* size of individual read operation */
		int  nreads,   /* number of read operations */
		void *buf)     /* result buffer */
{
    if (nreads <= 0)
	return 0;

    if (asid < -1 || asid > 255)
	return 0;

    __oldtrap = bsp_install_dbg_handler(err_trap);
    
    if (setjmp(__errjmp) == 0) {
	moveproc_t move_mem;
	int        incr;
	char       *src, *dest;

	switch (rsize) {
	  case 8:
	    if (asid < 0)
		move_mem = move_8;
	    else {
		__load_8a_insn &= 0xffffe01f;
		__load_8a_insn |= (asid << 5);
		move_mem = __load_8a;
		bsp_flush_icache(&__load_8a_insn, 4);
	    }
	    incr = 1;
	    break;
	  case 16:
	    if (asid < 0)
		move_mem = move_16;
	    else {
		__load_16a_insn &= 0xffffe01f;
		__load_16a_insn |= (asid << 5);
		move_mem = __load_16a;
		bsp_flush_icache(&__load_16a_insn, 4);
	    }
	    incr = 2;
	    break;
	  case 32:
	    if (asid < 0)
		move_mem = move_32;
	    else {
		__load_32a_insn &= 0xffffe01f;
		__load_32a_insn |= (asid << 5);
		move_mem = __load_32a;
		bsp_flush_icache(&__load_32a_insn, 4);
	    }
	    incr = 4;
	    break;
	  default:
	    (void)bsp_install_dbg_handler(__oldtrap);
	    return 0;
	}

	src = addr;
	dest = buf;

	for (__done = 0; __done < nreads; __done++) {
	    move_mem(src, dest);
	    src  += incr;
	    dest += incr;
	}
    }

    (void)bsp_install_dbg_handler(__oldtrap);
    return __done;
}


int
bsp_memory_write(void *addr,   /* start addr of memory to write */
		 int  asid,    /* address space id */
		 int  wsize,   /* size of individual write operation */
		 int  nwrites, /* number of write operations */
		 void *buf)    /* source buffer for write data */
{
    if (nwrites <= 0)
	return 0;

    if (asid < -1 || asid > 255)
	return 0;

    __oldtrap = bsp_install_dbg_handler(err_trap);
    
    if (setjmp(__errjmp) == 0) {
	moveproc_t move_mem;
	int        incr;
	char       *src, *dest;

	switch (wsize) {
	  case 8:
	    if (asid < 0)
		move_mem = move_8;
	    else {
		__store_8a_insn &= 0xffffe01f;
		__store_8a_insn |= (asid << 5);
		move_mem = __store_8a;
		bsp_flush_icache(&__store_8a_insn, 4);
	    }
	    incr = 1;
	    break;
	  case 16:
	    if (asid < 0)
		move_mem = move_16;
	    else {
		__store_16a_insn &= 0xffffe01f;
		__store_16a_insn |= (asid << 5);
		move_mem = __store_16a;
		bsp_flush_icache(&__store_16a_insn, 4);
	    }
	    incr = 2;
	    break;
	  case 32:
	    if (asid < 0)
		move_mem = move_32;
	    else {
		__store_32a_insn &= 0xffffe01f;
		__store_32a_insn |= (asid << 5);
		move_mem = __store_32a;
		bsp_flush_icache(&__store_32a_insn, 4);
	    }
	    incr = 4;
	    break;
	  default:
	    (void)bsp_install_dbg_handler(__oldtrap);
	    return 0;
	}

	src = buf;
	dest = addr;

	for (__done = 0; __done < nwrites; __done++) {
	    move_mem(src, dest);
	    bsp_flush_dcache(dest, incr);
	    src  += incr;
	    dest += incr;
	}
    }

    (void)bsp_install_dbg_handler(__oldtrap);
    return __done;
}

