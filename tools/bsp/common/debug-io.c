/*
 * debug-io.c -- BSP Debug Channel Interfaces.
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
#include <bsp/bsp.h>
#include "bsp_if.h"

static unsigned char __debug_ungetc;

void
bsp_debug_write(const char *p, int len)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    com->__write(com->ch_data, p, len);
}


int
bsp_debug_read(char *p, int len)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    if (len <= 0)
	return 0;

    if (__debug_ungetc) {
	*p = __debug_ungetc;
	__debug_ungetc = 0;
	return 1;
    }

    return com->__read(com->ch_data, p, len);
}


void
bsp_debug_putc(char ch)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    com->__putc(com->ch_data, ch);
}

int
bsp_debug_getc(void)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;
    int  ch;

    if (__debug_ungetc) {
	ch = __debug_ungetc;
	__debug_ungetc = 0;
    } else
	ch = com->__getc(com->ch_data);

    return ch;
}


void
bsp_debug_ungetc(char ch)
{
    __debug_ungetc = (unsigned char)ch;
}


int
bsp_debug_irq_disable(void)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    return com->__control(com->ch_data, COMMCTL_IRQ_DISABLE);
}


void
bsp_debug_irq_enable(void)
{
    struct bsp_comm_procs *com = bsp_shared_data->__debug_procs;

    com->__control(com->ch_data, COMMCTL_IRQ_ENABLE);
}


