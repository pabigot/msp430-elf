/* write.c -- write bytes to an output device.
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
#include <errno.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "bsp-trap.h"

void
bsp_console_interrupt_check(void)
{
    if (bsp_shared_data->__console_interrupt_flag) {
	bsp_shared_data->__console_interrupt_flag = 0;
	_bsp_trap(SYS_interrupt);
    }
}

/*
 * _write  -- write bytes to an output device.
 */
int
_write(int fd, const char *buf, int nbytes)
{
    int err;

    if (bsp_shared_data->version > 1)
	bsp_shared_data->__console_interrupt_flag = 0;

    err = _bsp_trap(SYS_write, fd, buf, nbytes);
    if (err)
	errno = err;

    if (bsp_shared_data->version > 1)
	bsp_console_interrupt_check();

    return err;
}
