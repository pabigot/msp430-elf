/* bsplog.c -- Debug log for internal BSP use.
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

#include <string.h>
#include <stdarg.h>
#include <bsp/bsp.h>

static char *logbuf_start;  /* start of log buffer */
static char *logbuf_end;    /* just past end of log buffer */
static char **logbuf_pp;    /* pointer to current log ptr */

#define MAX_LOGSTR 2048
static char tmp_buf[MAX_LOGSTR];

extern void DumpStatus(void);
/*
 * Dump the contents of the log.
 */
void
bsp_log_dump(void)
{
    char *p = *logbuf_pp;
#ifdef IRQ_FLAG
    IRQ_FLAG  flags;

    DISABLE_IRQ(flags);
#endif

    bsp_set_console_comm(0);

    /*    DumpStatus();*/

    bsp_printf("%s", p);
    *p = '\0';
    bsp_printf("%s", logbuf_start);

    for (p = logbuf_start; p <= logbuf_end; p++)
	*p = 0;

    *logbuf_pp = logbuf_start;

#ifdef IRQ_FLAG
    RESTORE_IRQ(flags);
#endif
}

/*
 * Initialize the log. If 'dump' is true, we're trying to dump
 * it after a post-crash reset.
 *
 * Note that the buffer passed in is an int* so that alignment
 * is good for the current pointer.
 */
void
bsp_log_init(int *buf, int nbytes, int dump)
{
    char *p;

    bsp_printf("bsp_log_init: buf[0x%x] len[%d] dump[%d]\n",
	       buf, nbytes, dump);

    logbuf_start = (char *)(buf + 1); /* leave room for ptr */
    logbuf_end   = ((char *)buf) + nbytes - 1;
    logbuf_pp    = (char **)buf;

    if (dump)
	bsp_log_dump();

    for (p = logbuf_start; p <= logbuf_end; p++)
	*p = 0;

    *logbuf_pp = logbuf_start;
}


static void
log_insert(char *src, int n)
{
    int     rem;
    char    *p;

    if (n <= 0)
	return;

    p = *logbuf_pp;

    rem = logbuf_end - p;

    if (rem < n) {
	memcpy(p, src, rem);
	n -= rem;
	src += rem;
	p = logbuf_start;
    }

    memcpy(p, src, n);

    *logbuf_pp = p + n;
}


void
bsp_log(char *fmt, ...)
{
    va_list ap;
    int     n;
#ifdef IRQ_FLAG
    IRQ_FLAG  flags;

    DISABLE_IRQ(flags);
#endif

#ifdef BSP_LOG_TICKS
    bsp_sprintf(tmp_buf, "%d: ", _bsp_ms_ticks());
    log_insert(tmp_buf, strlen(tmp_buf));
#endif

    va_start(ap, fmt);
    n = bsp_vsprintf(tmp_buf, fmt, ap);
    va_end(ap);

    log_insert(tmp_buf, n);

#ifdef IRQ_FLAG
    RESTORE_IRQ(flags);
#endif
}
