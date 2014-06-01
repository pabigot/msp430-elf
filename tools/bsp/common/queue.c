/*
 * queue.c -- General circular queue support.
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

#include "queue.h"


/*
 *  Initialize a queue to use the given buffer of size bytes.
 */
void
_bsp_queue_init(bsp_queue_t *q, char *buffer, int size)
{
    q->start = buffer;
    q->end   = buffer + size;
    q->rptr  = buffer;
    q->wptr  = buffer;
}


/*
 * Put a character into the queue. Don't worry about buffer
 * overflow, just discard the old characters.
 *
 * NB: It is the callers responsibility to ensure that the
 *     given queue cannot be accessed by an ISR while this
 *     function is executing.
 */
void
_bsp_enqueue (bsp_queue_t *q, char ch)
{
    *q->wptr++ = ch;
    if (q->wptr >= q->end)
	q->wptr = q->start;

    /* avoid "empty" condition on overflow */
    if (q->wptr == q->rptr)
	if (++q->rptr >= q->end)
	    q->rptr = q->start;
}


/*
 * Get next character from the queue. If the queue is empty, return -1.
 *
 * NB: It is the callers responsibility to ensure that the
 *     given queue cannot be accessed by an ISR while this
 *     function is executing.
 */
int
_bsp_dequeue (bsp_queue_t *q)
{
    int ch;

    if (q->rptr == q->wptr)
	return -1;

    ch = *q->rptr++ & 0xff;

    if (q->rptr >= q->end)
	q->rptr = q->start;

    return ch;
}

