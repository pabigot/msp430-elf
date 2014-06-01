/* sbrk.c -- allocate memory dynamically.
 * 
 * Copyright (c) 1998, 2000 Cygnus Support
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
#include "bsp-trap.h"
#include <errno.h>

extern char _end[];                /* _end is set in the linker command file */

char *heap_ptr;

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
char *
_sbrk (int nbytes)
{
    char        *base;

    if (!heap_ptr)
	heap_ptr = (char *)&_end;
    base = heap_ptr;
    heap_ptr += nbytes;

    return base;
/* FIXME: We really want to make sure we don't run out of RAM, but this
 *       isn't very portable.
 */
#if 0
  if ((RAMSIZE - heap_ptr - nbytes) >= 0) {
    base = heap_ptr;
    heap_ptr += nbytes;
    return (base);
  } else {
    errno = ENOMEM;
    return ((char *)-1);
  }
#endif
}
