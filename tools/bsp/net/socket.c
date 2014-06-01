/*
 * socket.c -- Polling TCP socket interface.
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
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "net.h"
#include "bsplog.h"


void
__skt_wait_for_connect(tcp_socket_t *s)
{
    BSPLOG(bsp_log("wait_for_connect start\n"));
    do {
	__tcp_listen(s, s->our_port);
	while (s->state != _ESTABLISHED && s->state != _CLOSED)
	    __tcp_poll();
    } while (s->state != _ESTABLISHED);
    BSPLOG(bsp_log("wait_for_connect done\n"));
}


/*
 * Initialize a socket for given port.
 */
void
__skt_init(tcp_socket_t *s, unsigned short port)
{
    s->state = _CLOSED;
    s->our_port = port;
}


#if 0
/*
 * Read up to 'len' bytes from the given socket.
 * Returns number of bytes read.
 */
int
__skt_read(tcp_socket_t *s, char *buf, int len)
{
    int n;
    
    if (len <= 0)
	return 0;
    
    while (1) {
	if (s->state == _CLOSE_WAIT)
	    __tcp_close(s);
	if (s->state == _CLOSED)
	    __skt_wait_for_connect(s);

	__tcp_poll();
	n = __tcp_read(s, buf, len);
	if (n > 0)
	    return n;
    }
    return 0; /* never reached */
}
#endif


/*
 * Read up to 'len' bytes from the given socket
 * without blocking. Returns number of bytes read.
 */
int
__skt_read(tcp_socket_t *s, char *buf, int len)
{
    __tcp_poll();

    if (s->state == _CLOSE_WAIT) {
	/* if we've sent all data, __tcp_close won't block */
	if (s->data_bytes == 0)
	    __tcp_close(s);
	return 0;
    }

    if (s->state == _CLOSED) {
#if 0 /* let higher level see the close and handle it */
	__tcp_listen(s, s->our_port);
	__tcp_poll();
#endif
	return 0;
    }

    if (len <= 0)
	return 0;
    
    return __tcp_read(s, buf, len);
}


/*
 * Write 'len' bytes to the given socket.
 * Returns number of bytes written.
 */
int
__skt_write(tcp_socket_t *s, char *buf, int len)
{
    int n, nleft;

    if (len < 0)
	return 0;

    if (len == 0) {
	__tcp_drain(s);
	return 0;
    }

    nleft = len;
    while (1) {
	if (s->state == _CLOSE_WAIT)
	    __tcp_close(s);
	if (s->state == _CLOSED) {
#if 0 /* let higher level see the close */
	    __skt_wait_for_connect(s);
#else
	    return len - nleft;
#endif
	}

	n = __tcp_write(s, buf, nleft);
	if (n > 0) {
	    nleft -= n;
	    if (nleft == 0)
		return len;
	    buf += n;
	}
	__tcp_poll();
    }
    return 0; /* never reached */
}

