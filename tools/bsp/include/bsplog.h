/* bsplog.h -- Debug log for internal BSP use.
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

extern void bsp_log_dump(void);

/*
 * Initialize the log. If 'dump' is true, we're trying to dump
 * it after a post-crash reset.
 *
 * Note that the buffer passed in is an int* so that alignment
 * is good for the current pointer.
 */
extern void bsp_log_init(int *buf, int nbytes, int dump);


extern void bsp_log(char *fmt, ...);

#ifdef BSP_LOG
#define BSPLOG(x) x
#else
#define BSPLOG(x)
#endif
