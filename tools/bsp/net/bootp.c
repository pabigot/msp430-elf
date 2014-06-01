/*
 * bootp.c -- Polling BOOTP for Cygnus BSP
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
#include <string.h>
#include "bsp_if.h"
#include "net.h"


#define SHOULD_BE_RANDOM  0x12345555

/* How many milliseconds to wait before retrying the request */
#define RETRY_TIME  2000

static void
bootp_handler(udp_socket_t *skt, char *buf, int len,
	      ip_route_t *src_route, word src_port)
{
    bootp_header_t *b;

    b = (bootp_header_t *)buf;

    if (b->opcode == BOOTP_OP_REPLY && 
	!memcmp(b->client_hw, __local_enet_addr, 6)) {
	/* got a response */
	memcpy(__local_ip_addr, b->your_ip, 4);
#ifdef TFTP_BOOT
	memcpy(__tftp_gateway_ip, b->gateway_ip, 4);
	memcpy(__tftp_server_ip, b->server_ip, 4);
	strcpy(__tftp_filename, b->boot_file);
#endif
    }
}


/*
 * Find our IP address and copy to __local_ip_addr.
 * Return zero if successful, -1 if not.
 */
int
__bootp_find_local_ip(void)
{
    udp_socket_t udp_skt;
    bootp_header_t b;
    ip_route_t     r;
    int            retry;
    unsigned long  start;

    memset(&b, 0, sizeof(b));

    b.opcode = BOOTP_OP_REQUEST;
    b.hw_type = ARP_HW_ETHER;
    b.hw_len = 6;
    b.trans_id = SHOULD_BE_RANDOM;

    __local_ip_addr[0] = 0;
    __local_ip_addr[1] = 0;
    __local_ip_addr[2] = 0;
    __local_ip_addr[3] = 0;

    memcpy(b.client_hw, __local_enet_addr, 6);

    /* fill out route for a broadcast */
    r.ip_addr[0] = 255;
    r.ip_addr[1] = 255;
    r.ip_addr[2] = 255;
    r.ip_addr[3] = 255;
    r.enet_addr[0] = 255;
    r.enet_addr[1] = 255;
    r.enet_addr[2] = 255;
    r.enet_addr[3] = 255;
    r.enet_addr[4] = 255;
    r.enet_addr[5] = 255;

    /* setup a socket listener for bootp replies */
    __udp_install_listener(&udp_skt, 68, bootp_handler);

    retry = 3;
    while (retry-- > 0) {
	start = MS_TICKS();

	__udp_send((char *)&b, sizeof(b), &r, 67, 68);

	do {
	    __enet_poll();
	    if (__local_ip_addr[0] || __local_ip_addr[1] ||
		__local_ip_addr[2] || __local_ip_addr[3]) {
		/* success */
		__udp_remove_listener(68);
		return 0;
	    }
	} while ((MS_TICKS() - start) < RETRY_TIME);
    }

    /* timed out */
    __udp_remove_listener(68);
    return -1;
}


