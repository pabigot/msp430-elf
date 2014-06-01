/* times.c
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
#include <_ansi.h>
#include <sys/times.h>
#include "bsp-trap.h"

/* Return a clock that ticks at 100Hz. */
clock_t
_times(struct tms * tp)
{
    clock_t utime;
#ifdef HAVE_BSP_CLOCK
    int err;
    err = _bsp_trap(SYS_utime, &utime);
    if (err)
	utime = 0;
#else
    utime = 0;
#endif

    if (tp) {
	tp->tms_utime = utime;
	tp->tms_stime = 0;
	tp->tms_cutime = 0;
	tp->tms_cstime = 0;
    }

    return utime;
}
