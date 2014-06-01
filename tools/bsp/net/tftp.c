/*
 * tftp.c -- Polling TFTP for Cygnus BSP
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

#define TFTP_TIMEOUT 5000    /* 5 seconds */

#define MY_TFTP_PORT 1100

#define TFTP_NAMEMAX 500

#define MODE_BINARY "octet"
#define MODE_ASCII  "netascii"

#define TFTP_BLOCKSIZE 512

typedef struct {
    udp_socket_t   skt;
    void           *xfer_ptr
    int            xfer_cnt;
    int            xfer_max;
    timer_t        timer;
    ip_route_t     route;
    unsigned short buf[516/sizeof(short)];
    short          buflen;
    short          done;
    unsigned short my_port;
    unsigned short his_port;
    unsigned short next_block;
} tftp_conn_t;

static void do_retrans(void *p);

ip_addr_t __tftp_server_ip  = { 0, 0, 0, 0 };
ip_addr_t __tftp_gateway_ip = { 0, 0, 0, 0 };
char      __tftp_filename[128];

static void
read_handler(udp_socket_t *skt, char *buf, int len,
	     ip_route_t *src_route, word src_port)
{
    tftp_conn_t   *conn = (tftp_conn_t *)skt;
    tftp_header_t *t = (tftp_header_t *)buf;
    unsigned short errbuf[4];

    /* ignore it if we don't have an OPCODE at least */
    if (len < 2)
	return;

    /* skip opcode*/
    len -= 2;
    buf += 2;

    switch (ntohs(t->opcode)) {
      case TFTP_OP_DATA:
	if (conn->xfer_cnt == 0) {
	    conn->his_port = src_port;
	} else if (src_port != conn->his_port) {
	    /* send error message to originator */
	    t = (tftp_header_t *)errbuf;
	    t->opcode = htons(TFTP_OP_ERR);
	    t->u.error_nr = htons(TFTP_ERR_BADID);
	    t->u.name[3] = '\0';
	    __udp_send((char *)t, sizeof(tftp_header_t) + 1, src_route, src_port, conn->my_port);
	}

	/* always refresh route info */
	conn->route = *src_route;

	/* ignore it if we don't have a block_nr */
	if (len < 2)
	    return;
	/* skip block_nr */
	len -= 2;
	buf += 2;

	if (ntohs(t->u.block_nr) == conn->next_block) {
	    /* got the next (maybe last) block */
	    int room;

	    room = conn->xfer_max - conn->xfer_cnt;
	    if (len > room)
		len = room;

	    memcpy(conn->xfer_ptr, buf, len);
	    conn->xfer_ptr   += len;
	    conn->xfer_cnt   += len;
	    
	    /* ACK the block */
	    t = (tftp_header_t *)conn.buf;
	    t->opcode = htons(TFTP_OP_ACK);
	    t->u.block_nr = htons(conn->next_block);
	    conn->next_block += 1;
	    conn->buflen = sizeof(tftp_header_t);

	    /* cancel old timer */
	    __timer_cancel(&conn->timer);

	    if (len < TFTP_BLOCK_SIZE) 
		conn->done = 1;
	    else
		/* start new timer */
		__timer_set(&conn->timer, TFTP_TIMEOUT, do_retrans, conn);

	    /* send ACK packet */
	    __udp_send(conn->buf, conn->buflen, &conn->route, conn->his_port, conn->my_port);
	}
	break;
      case TFTP_OP_ERR:
	/* shutdown connection */
	__timer_cancel(&conn->timer);
	conn.done = -1;
	break;
      default:
	break;
    }
}


int
__tftp_read(ip_route_t *r, const char *filename, void *buffer, int buflen)
{
    tftp_conn_t    conn;
    tftp_header_t  *t;
    int            rval = -1;

    memset(conn, 0, sizeof(conn));

    conn.my_port = MY_TFTP_PORT;
    conn.his_port = TFTP_SERVER_PORT;

    t = (tftp_header_t *)conn.buf;

    t->opcode = htons(TFTP_OP_RRQ);
    strncpy(t->u.name, filename, TFTP_NAMEMAX);
    strcat(t->u.name, MODE_BINARY);

    conn.route = *r;

    /* setup a socket listener for tftp replies */
    __udp_install_listener(&conn.udp_skt, conn.my_port, read_handler);

    conn.xfer_ptr = buffer;
    conn.xfer_max = buflen;
    conn.xfer_cnt = 0;
    conn.done = 0;
    conn.next_block = 1;
    conn.buflen = sizeof(tftp_header_t) + strlen(filename) + strlen(MODE_BINARY);

    /* set timer and send read request */
    __timer_set(&t->timer, TFTP_TIMEOUT, do_retrans, t);
    __udp_send(t, conn.buflen, &conn.route, conn.his_port, conn.my_port);

    while (!conn.done) {
	__enet_poll();
	__timer_poll();
    }

    if (conn.done > 0) {
	/* success */
	rval = conn.xfer_cnt;
    }

    __udp_remove_listener(conn.myport);
    return rval;
}


static void
do_retrans(void *p)
{
    tftp_conn_t *conn = (tftp_conn_t *)p;

    /* reset timer */
    __timer_set(&conn->timer, TFTP_TIMEOUT, do_retrans, conn);

    /* resend last packet */
    __udp_send(conn->buf, conn->buflen, &conn->route, conn->his_port, conn->my_port);
}

int
__tftp_boot(struct bsp_comm_procs *procs)
{
    if (!_bsp_net_init(proc))
	return 0;


}
