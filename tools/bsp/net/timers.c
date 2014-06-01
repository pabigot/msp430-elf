/*
 * timers.c -- Simple timer management for polled network stack.
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
#include "net.h"
#include "bsplog.h"

static timer_t *tmr_list;


/*
 * Set a timer. Caller is responsible for providing the timer_t struct.
 */
void
__timer_set(timer_t *t, unsigned long delay,
	    tmr_handler_t handler, void *user_data)
{
    timer_t *p;

    t->delay = delay;
    t->start = MS_TICKS();
    t->handler = handler;
    t->user_data = user_data;

    BSPLOG(bsp_log("__timer_set: t[%x] start[%d] delay[%d].\n",
		   t, t->start, t->delay));

    for (p = tmr_list; p; p = p->next)
	if (p == t) {
	    BSPLOG(bsp_log("__timer_set: dup entry.\n"));
	    return;
	}

    t->next = tmr_list;
    tmr_list = t;
}


/*
 * Remove a given timer from timer list.
 */
void
__timer_cancel(timer_t *t)
{
    timer_t *prev, *p;

    BSPLOG(bsp_log("__timer_cancel: t[%x].\n", t));

    for (prev = NULL, p = tmr_list; p; prev = p, p = p->next)
	if (p == t) {
	    BSPLOG(bsp_log("found: start[%d] delay[%d].\n",
			   t->start, t->delay));
	    if (prev)
		prev->next = p->next;
	    else
		tmr_list = p->next;
	    return;
	}
}


/*
 * Poll timer list for timer expirations.
 */
void
__timer_poll(void)
{
    timer_t *prev, *t;

    prev = NULL;
    t = tmr_list;
    while (t) {
	if ((MS_TICKS() - t->start) >= t->delay) {
	    BSPLOG(bsp_log("_timer_poll: expired [%x].\n", t));

	    /* remove it before calling handler */
	    if (prev)
		prev->next = t->next;
	    else
		tmr_list = t->next;
	    /* now, call the handler */
	    t->handler(t->user_data);
	    
	    /*
	     * handler may be time consuming, so start
	     * from beginning of list.
	     */
	    prev = NULL;
	    t = tmr_list;
	} else {
	    prev = t;
	    t = t->next;
	}
    }
}
