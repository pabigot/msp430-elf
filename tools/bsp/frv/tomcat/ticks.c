/*
 * ticks.c -- Millisecond tick count for FR-V eval board.
 *
 * Copyright (c) 1999, 2000 Cygnus Support
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
#include <bsp/cpu.h>
#include <time.h>
#include "fr500.h"
#include "mb86941.h"


static unsigned long ticks;
static unsigned short last_count;


unsigned long
_bsp_ms_ticks(void)
{
    unsigned short current;
    unsigned short diff;

    current = mb86941_read(MB_BASE, MB86941_CNT1);

    /* its a down counter, so use "last - current" */
    diff = last_count - current;
    ticks += diff;
    last_count = current;

    return ticks;
}

/*
 * Tomcat uses 20MHz clock
 */
#define TIMER_HZ 20000000


void
_bsp_init_ms_timer(void)
{
    unsigned int clk;

    /* make sure its disabled */
    mb86941_write(MB_BASE, MB86941_TCR1, 0);

    /*
     * Timer internal clock is divide by two.
     * Prescaler divides by 128
     */
    clk = TIMER_HZ / 256;

    clk = clk / 1000;
    if (clk > 255)
	clk = 255;

    mb86941_write(MB_BASE, MB86941_PRS1, PRS_ODIV128 | clk);
    mb86941_write(MB_BASE, MB86941_TCR1, TCR_CE|TCR_CLKPRS|TCR_OUTC3|TCR_SQWAVE);
    mb86941_write(MB_BASE, MB86941_RELOAD1, 0xffff);
}


#ifdef HAVE_BSP_CLOCK
clock_t _bsp_clock;

/*
 * Initialize an interrupt driven timer to provide a system clock feature.
 * We use 1000 CLOCKS_PER_SEC from <time.h>
 */
void
_bsp_init_clock_timer(void)
{
    unsigned int cnt;

    _bsp_clock = 0;

    /* make sure its disabled */
    mb86941_write(MB_BASE, MB86941_TCR2, 0);

    /*
     * Timer internal clock is divide by two.
     */
    cnt = TIMER_HZ / 2;
    cnt = cnt / CLOCKS_PER_SEC;

    mb86941_write(MB_BASE, MB86941_TCR2, TCR_CE|TCR_CLKINT|TCR_OUTLOW|TCR_PER_INT);
    mb86941_write(MB_BASE, MB86941_RELOAD2, cnt);
}
#endif /* HAVE_BSP_CLOCK */
