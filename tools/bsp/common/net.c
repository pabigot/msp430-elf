/*
 * net.c -- Glue for TCP debug channel.
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

#include <stdlib.h>
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "net.h"
#include "gdb.h"
#include "bsplog.h"

extern int __bootp_find_local_ip(void);

#ifndef TFTP_BOOT
static tcp_socket_t skt;
static unsigned char sbuf[512];
static unsigned char *sptr;
static int  scnt;
static int  report_close;

static int dbg_port;

static int
close_check(void *s)
{
    if (__skt_is_closed(s)) {
	BSPLOG(bsp_log("net socket closed. report[%d]\n", report_close));
	if (report_close) {
	    report_close = 0;
	    _bsp_dbg_connect_abort();
	    return 1;
	}
	__skt_wait_for_connect(s);
	report_close = 1;
    }
    return 0;
}

static void
net_putc(void *s, char ch)
{
    while (__skt_write(s, &ch, 1) != 1)
	if (close_check(s))
	    break;
}

static int
net_getc(void *s)
{
    if (scnt <= 0) {
	sptr = sbuf;
	while ((scnt = __skt_read(s, (char *)sbuf, sizeof(sbuf))) <= 0)
	    if (close_check(s))
		return -1;
    }
    --scnt;
    return *sptr++;
}

static void
net_write(void *s, const char *buf, int len)
{
    while (__skt_write(s, (char *)buf, len) != len)
	if (close_check(s))
	    break;;
}

static int
net_read(void *s, char *buf, int len)
{
    int rval;

    rval = __skt_read(s, buf, len);
    close_check(s);

    return rval;
}


/*
 * The network channel descriptor.
 */
static struct bsp_comm_channel net_channel = {
    { "Network", BSP_COMM_ENET, BSP_PROTO_TCP },
    { (void*)&skt, net_write, net_read,
      net_putc, net_getc, NULL }
};


void
_bsp_net_set_debug_port(int portnum)
{
    dbg_port = portnum;
}

#endif

/*
 * This is the initial interface into the networking support.
 * Calling this from board-specific startup code will pull in
 * network support.
 */
int
_bsp_net_init(struct bsp_comm_procs *procs)
{
    _bsp_install_enet_driver(procs);

    __pktbuf_init();

#ifndef USE_SLIP
    /* Use bootp if the ip address isn't already set up */
    if (__local_ip_addr[0] == 0 && __local_ip_addr[1] == 0 &&
	__local_ip_addr[2] == 0 && __local_ip_addr[3] == 0) {
	if (__bootp_find_local_ip() < 0) {
	    bsp_printf("BOOTP failed.\n");
	    _bsp_install_enet_driver(NULL);
	    return 0;
	}
	bsp_printf("BOOTP got %d.%d.%d.%d\n",
		   __local_ip_addr[0], __local_ip_addr[1],
		   __local_ip_addr[2], __local_ip_addr[3]);
    }
#endif

#ifndef TFTP_BOOT
    __skt_init(&skt, (dbg_port > 0) ? dbg_port : GDB_TCP_SOCKET);
    
    net_channel.procs.__control = procs->__control;

    _bsp_net_channel = &net_channel;
#endif

    return 1;
}
