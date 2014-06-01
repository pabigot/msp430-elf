/*
 * sprintf.c -- Light-weight BSP sprintf.
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

static char *str_ptr;

static void
str_putc(char ch)
{
    *str_ptr++ = ch;
}

int
bsp_vsprintf(char *str, const char *fmt, va_list ap)
{
    str_ptr = str;
    __vprintf(str_putc, fmt, ap);
    *str_ptr = '\0';
    return str_ptr - str;
}


void
bsp_sprintf(char *str, const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    bsp_vsprintf(str, fmt, ap);
    va_end (ap);
}


