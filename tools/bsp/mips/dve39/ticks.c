/*
 * ticks.c -- Millisecond tick count for dve3900/20 eval board.
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
#include <bsp/dve39.h>

#define CNT_MASK 0xfffffff

unsigned int
__usec_count(void)
{
    unsigned u;
    volatile unsigned *cnt = (volatile unsigned *)CSR13;
    volatile unsigned *iclr = (volatile unsigned *)CSR23;
    
    u = *cnt & CNT_MASK;
    *(volatile unsigned char *)cnt = 0x80;
    
    *iclr = 0x00100000;

    return u;
}


static unsigned long ticks;
static unsigned int  last_count;

unsigned long
_bsp_ms_ticks(void)
{
    unsigned int        current = __usec_count();
    static unsigned int diff = 0;

    /* its an up counter, so use "current - last" */
    diff += ((current - last_count) & CNT_MASK);

    if (diff >= 1000) {
	ticks += (diff / 1000);
	diff = (diff % 1000);
    }

    last_count = current;

    return ticks;
}

