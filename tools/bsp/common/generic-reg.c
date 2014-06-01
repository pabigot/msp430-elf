/*
 * generic-reg.c -- Generic support for read/write of CPU registers. Some
 *                  targets may need to provide their own version.
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
#include <stdlib.h>
#include <string.h>
#include <bsp/bsp.h>

void
bsp_set_register(int regno, void *saved_regs, void *val)
{
    memcpy((char *)saved_regs + bsp_regbyte(regno), val, bsp_regsize(regno));
}


void
bsp_get_register(int regno, void *saved_regs, void *val)
{
    memcpy(val, (char *)saved_regs + bsp_regbyte(regno), bsp_regsize(regno));
}

