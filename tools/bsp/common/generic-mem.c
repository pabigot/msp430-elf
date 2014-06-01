/*
 * mem.c -- Generic support for safe memory read/write. Some targets may need
 *          to provide their own version.
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
 */
#include <stdlib.h>
#include <setjmp.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>

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

#ifdef MOVE_64
static void
move_64(void *src, void *dest)
{
    MOVE_64(src,dest);
}
#endif

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

    __oldtrap = bsp_install_dbg_handler(err_trap);
    
    if (setjmp(__errjmp) == 0) {
	moveproc_t move_mem;
	int        incr;
	char       *src, *dest;

	switch (rsize) {
	  case 8:
	    move_mem = move_8;
	    incr = 1;
	    break;
	  case 16:
	    move_mem = move_16;
	    incr = 2;
	    break;
	  case 32:
	    move_mem = move_32;
	    incr = 4;
	    break;
#ifdef MOVE_64
	  case 64:
	    move_mem = move_64;
	    incr = 8;
	    break;
#endif
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


int bsp_memory_write(void *addr,   /* start addr of memory to write */
                     int  asid,    /* address space id */
                     int  wsize,   /* size of individual write operation */
                     int  nwrites, /* number of write operations */
                     void *buf)    /* source buffer for write data */
{
    if (nwrites <= 0)
	return 0;

    __oldtrap = bsp_install_dbg_handler(err_trap);
    
    if (setjmp(__errjmp) == 0) {
	moveproc_t move_mem;
	int        incr;
	char       *src, *dest;

	switch (wsize) {
	  case 8:
	    move_mem = move_8;
	    incr = 1;
	    break;
	  case 16:
	    move_mem = move_16;
	    incr = 2;
	    break;
	  case 32:
	    move_mem = move_32;
	    incr = 4;
	    break;
#ifdef MOVE_64
	  case 64:
	    move_mem = move_64;
	    incr = 8;
	    break;
#endif
	  default:
	    (void)bsp_install_dbg_handler(__oldtrap);
	    return 0;
	}

	src = buf;
	dest = addr;

	for (__done = 0; __done < nwrites; __done++) {
	    move_mem(src, dest);
	    src  += incr;
	    dest += incr;
	}
    }

    (void)bsp_install_dbg_handler(__oldtrap);
    return __done;
}
