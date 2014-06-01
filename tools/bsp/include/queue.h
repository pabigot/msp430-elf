/*
 * queue.h -- General circular queue support.
 *
 * Copyright (c) 1998 Cygnus Support
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

/*
 * The queue structure.
 */
typedef struct {
    char	  *start;
    char	  *end;
    volatile char *rptr;
    volatile char *wptr;
} bsp_queue_t;

    
/*
 *  Initialize a queue to use the given buffer of size bytes.
 */
extern void _bsp_queue_init(bsp_queue_t *q, char *buffer, int size);

/*
 * Put a character into the queue. Don't worry about buffer
 * overflow, just discard the old characters.
 *
 * NB: It is the callers responsibility to ensure that the
 *     given queue cannot be accessed by an ISR while this
 *     function is executing.
 */
extern void _bsp_enqueue (bsp_queue_t *q, char ch);

/*
 * Get next character from the queue. If the queue is empty, return -1.
 *
 * NB: It is the callers responsibility to ensure that the
 *     given queue cannot be accessed by an ISR while this
 *     function is executing.
 */
extern int _bsp_dequeue (bsp_queue_t *q);

