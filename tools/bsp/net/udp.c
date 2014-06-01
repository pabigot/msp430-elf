/*
 * udp.c -- Polling UDP for Cygnus BSP
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
#include <string.h>
#include "bsp_if.h"
#include "net.h"

#if UDP_STATS
static int udp_rx_total;
static int udp_rx_handled;
static int udp_rx_cksum;
static int udp_rx_dropped;
#endif

#define MAX_UDP_DATA (ETH_MAX_PKTLEN - (sizeof(eth_header_t) + \
					sizeof(ip_header_t)  + \
					sizeof(udp_header_t)))

/*
 * A major assumption is that only a very small number of sockets will
 * active, so a simple linear search of those sockets is acceptible.
 */
static udp_socket_t *udp_list;


/*
 * Install a handler for incoming udp packets.
 * Caller provides the udp_socket_t structure.
 * Returns zero if successful, -1 if socket is already used.
 */
int
__udp_install_listener(udp_socket_t *s, word port, udp_handler_t handler)
{
    udp_socket_t *p;

    /*
     * Make sure we only have one handler per port.
     */
    for (p = udp_list; p; p = p->next)
	if (p->our_port == port)
	    return -1;
    
    s->our_port = htons(port);
    s->handler = handler;
    s->next = udp_list;
    udp_list = s;

    return 0;
}


/*
 * Remove the handler for the given socket.
 */
void
__udp_remove_listener(word port)
{
    udp_socket_t *prev, *s;

    for (prev = NULL, s = udp_list; s; prev = s, s = s->next)
	if (s->our_port == htons(port)) {
	    if (prev)
		prev->next = s->next;
	    else
		udp_list = s->next;
	}
}


/*
 * Handle incoming UDP packets.
 */
void
__udp_handler(pktbuf_t *pkt, ip_route_t *r)
{
    udp_header_t *udp = pkt->udp_hdr;
    ip_header_t  *ip = pkt->ip_hdr;
    udp_socket_t *s;

    if (udp->checksum == 0xffff)
	udp->checksum = 0;

    /* copy length for pseudo sum calculation */
    ip->length = udp->length;

    if (__sum((word *)udp, ntohs(udp->length), __pseudo_sum(ip)) == 0) {
	for (s = udp_list; s; s = s->next) {
	    if (s->our_port == udp->dest_port) {
		(*s->handler)(s, ((char *)udp) + sizeof(udp_header_t),
			      ntohs(udp->length) - sizeof(udp_header_t),
			      r, ntohs(udp->src_port));
		break;
	    }
	}
    }
    __pktbuf_free(pkt);
}


/*
 * Send a UDP packet.
 */
void
__udp_send(char *buf, int len, ip_route_t *dest_ip,
	   word dest_port, word src_port)
{
    pktbuf_t *pkt;
    udp_header_t *udp;
    ip_header_t *ip;


    /* dumb */
    if (len > MAX_UDP_DATA)
	return;

    /* just drop it if can't get a buffer */
    if ((pkt = __pktbuf_alloc(ETH_MAX_PKTLEN)) == NULL)
	return;

    udp = pkt->udp_hdr;
    ip = pkt->ip_hdr;

    pkt->pkt_bytes = len + sizeof(udp_header_t);

    udp->src_port = htons(src_port);
    udp->dest_port = htons(dest_port);
    udp->length = htons(pkt->pkt_bytes);
    udp->checksum = 0;

    memcpy(((char *)udp) + sizeof(udp_header_t), buf, len);

    /* fill in some pseudo-header fields */
    memcpy(ip->source, __local_ip_addr, sizeof(ip_addr_t));
    memcpy(ip->destination, dest_ip->ip_addr, sizeof(ip_addr_t));
    ip->protocol = IP_PROTO_UDP;
    ip->length = udp->length;

    udp->checksum = __sum((word *)udp, pkt->pkt_bytes, __pseudo_sum(ip));

    __ip_send(pkt, IP_PROTO_UDP, dest_ip);

    __pktbuf_free(pkt);
}
