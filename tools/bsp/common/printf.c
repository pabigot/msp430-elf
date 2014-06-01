/*
 * printf.c -- Light-weight BSP printf.
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

#include <bsp/bsp.h>
#include "bsp_if.h"


void
bsp_vprintf(const char *fmt, va_list ap)
{
    __vprintf(bsp_console_putc, fmt, ap);
}


void
bsp_dvprintf(const char *fmt, va_list ap)
{
    __vprintf(bsp_debug_putc, fmt, ap);
}


void
bsp_printf(const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    __vprintf(bsp_console_putc, fmt, ap);
    va_end (ap);
}


void
bsp_dprintf(const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    __vprintf(bsp_debug_putc, fmt, ap);
    va_end (ap);
}



