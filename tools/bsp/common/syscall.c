/*
 * syscall.c -- Minimal generic syscall support.
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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

#include <errno.h>
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include <time.h>
#include <sys/param.h>
#include "bsp_if.h"
#include "syscall.h"

/*
 * read  -- read bytes from the serial port. Ignore fd, since
 *          we only have stdin.
 */
static int
sys_read(int fd, char *buf, int nbytes)
{
    int i = 0;

    for (i = 0; i < nbytes; i++) {
	*(buf + i) = bsp_console_getc();
	if ((*(buf + i) == '\n') || (*(buf + i) == '\r')) {
	    (*(buf + i + 1)) = 0;
	    break;
	}
    }
    return (i);
}


/*
 * write -- write bytes to the serial port. Ignore fd, since
 *          stdout and stderr are the same. Since we have no filesystem,
 *          open will only return an error.
 */
static int
sys_write(int fd, char *buf, int nbytes)
{
#define WBUFSIZE  256
    char ch, lbuf[WBUFSIZE];
    int  i, tosend;

    tosend = nbytes;

    while (tosend > 0) {
	for (i = 0; tosend > 0 && i < (WBUFSIZE-2); tosend--) {
	    ch = *buf++;
	    if (ch == '\n')
		lbuf[i++] = '\r';
	    lbuf[i++] = ch;
	}
	bsp_console_write(lbuf, i);
    }

    return (nbytes);
}


/*
 * open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
 */
static int
sys_open (const char *buf, int flags, int mode)
{
    return (-EIO);
}


/*
 * close -- We don't need to do anything, but pretend we did.
 */
static int
sys_close(int fd)
{
    return (0);
}


/*
 * lseek --  Since a serial port is non-seekable, we return an error.
 */
static int
sys_lseek(int fd,  int offset, int whence)
{
    return (-EIO);
}


static int
sys_utime(clock_t *p)
{
#ifdef HAVE_BSP_CLOCK
    extern clock_t _bsp_clock;

    /* BSP clock runs at CLOCKS_PER_SEC. Convert to HZ */
    if (p)
	*p = (_bsp_clock * HZ) / CLOCKS_PER_SEC;
#else
    if (p)
	*p = 0;
#endif
    return 0;
}


/*
 *  Generic syscall handler.
 *
 *  Returns 0 if syscall number is not handled by this
 *  module, 1 otherwise. This allows applications to
 *  extend the syscall handler by using exception chaining.
 */
int
_bsp_do_syscall(int func,		/* syscall function number */
		long arg1, long arg2,	/* up to four args.        */
		long arg3, long arg4,
		int *retval)		/* syscall return value    */
{
    int err = 0;

    switch (func) {

      case SYS_read:
	err = sys_read((int)arg1, (char *)arg2, (int)arg3);
	break;

      case SYS_write:
	err = sys_write((int)arg1, (char *)arg2, (int)arg3);
	break;

      case SYS_open:
	err = sys_open((const char *)arg1, (int)arg2, (int)arg3);
	break;

      case SYS_close:
	err = sys_close((int)arg1);
	break;

      case SYS_lseek:
	err = sys_lseek((int)arg1, (int)arg2, (int)arg3);
	break;

      case SYS_utime:
	err = sys_utime((clock_t *)arg1);
	break;

      case BSP_GET_SHARED:
	*(bsp_shared_t **)arg1 = bsp_shared_data;
	break;

      default:
	return 0;
    }    

    *retval = err;
    return 1;
}


