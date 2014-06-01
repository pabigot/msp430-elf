/*
 * console-io.c -- BSP Console Channel Interfaces.
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

static unsigned char __console_ungetc;

void
bsp_console_write(const char *p, int len)
{
    struct bsp_comm_procs *com;

    if ((com = bsp_shared_data->__console_procs) != NULL)
	com->__write(com->ch_data, p, len);
    else
	bsp_debug_write(p, len);
}


int
bsp_console_read(char *p, int len)
{
    struct bsp_comm_procs *com;

    if (len <= 0)
	return 0;

    if ((com = bsp_shared_data->__console_procs) != NULL) {
	if (__console_ungetc) {
	    *p = __console_ungetc;
	    __console_ungetc = 0;
	    return 1;
	}
	return com->__read(com->ch_data, p, len);
    } else
	return bsp_debug_read(p, len);
}

/*#define PRINTABLE_ONLY*/
#ifdef PRINTABLE_ONLY
#include <ctype.h>
#endif /* PRINTABLE_ONLY */

void
bsp_console_putc(char ch)
{
    struct bsp_comm_procs *com;

#ifdef PRINTABLE_ONLY
    if ((!isprint(ch)) && (!isspace(ch)))
        ch = '.';
#endif /* PRINTABLE_ONLY */

    if ((com = bsp_shared_data->__console_procs) != NULL)
	com->__putc(com->ch_data, ch);
    else
	bsp_debug_putc(ch);
}

int
bsp_console_getc(void)
{
    struct bsp_comm_procs *com;
    int    ch;

    if ((com = bsp_shared_data->__console_procs) != NULL) {
	if (__console_ungetc) {
	    ch = __console_ungetc;
	    __console_ungetc = 0;
	    return ch;
	}
	return com->__getc(com->ch_data);
    } else
	return bsp_debug_getc();
}


void
bsp_console_ungetc(char ch)
{
    if (bsp_shared_data->__console_procs != NULL)
	__console_ungetc = (unsigned char)ch;
    else
	bsp_debug_ungetc(ch);
}


