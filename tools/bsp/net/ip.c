/*
 * ip.c -- Polling IP for Cygnus BSP
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
#include <string.h>
#include "bsp_if.h"
#include "net.h"

ip_addr_t __local_ip_addr = { 0, 0, 0, 0 };

static word ip_ident;

#if IP_STATS
#endif


/*
 * Match given IP address to our address.
 * Check for broadcast matches as well.
 */
static int
ip_addr_match(ip_addr_t addr)
{
    if (addr[0] == 255 && addr[1] == 255 && addr[2] == 255 && addr[3] == 255)
	return 1;

    if (!memcmp(addr, __local_ip_addr, sizeof(ip_addr_t)))
	return 1;

    /*
     * Consider it an address match if we haven't gotten our IP address yet.
     * Some DHCP servers will address IP packets to the assigned address
     * instead of a IP broadcast address.
     */
    if (__local_ip_addr[0] == 0 && __local_ip_addr[1] == 0 &&
	__local_ip_addr[2] == 0 && __local_ip_addr[3] == 0)
	return 1;

    return 0;
}


extern void __tcp_handler(pktbuf_t *, ip_route_t *);

/*
 * Handle IP packets coming from the polled ethernet interface.
 */
void
__ip_handler(pktbuf_t *pkt, enet_addr_t src_enet_addr)
{
    ip_header_t *ip = pkt->ip_hdr;
    ip_route_t  r;
    int         hdr_bytes;

    /* first make sure its ours and has a good checksum. */
    if (!ip_addr_match(ip->destination) ||
	__sum((word *)ip, ip->hdr_len << 2, 0) != 0) {
	__pktbuf_free(pkt);
	return;
    }

    memcpy(r.ip_addr, ip->source, sizeof(ip_addr_t));
    memcpy(r.enet_addr, src_enet_addr, sizeof(enet_addr_t));

    hdr_bytes = ip->hdr_len << 2;
    pkt->pkt_bytes = ntohs(ip->length) - hdr_bytes;

    switch (ip->protocol) {

#if NET_SUPPORT_ICMP
      case IP_PROTO_ICMP:
	pkt->icmp_hdr = (icmp_header_t *)(((char *)ip) + hdr_bytes);
	__icmp_handler(pkt, &r);
	break;
#endif

#if NET_SUPPORT_TCP
      case IP_PROTO_TCP:
	pkt->tcp_hdr = (tcp_header_t *)(((char *)ip) + hdr_bytes);
	__tcp_handler(pkt, &r);
	break;
#endif

#if NET_SUPPORT_UDP
      case IP_PROTO_UDP:
	pkt->udp_hdr = (udp_header_t *)(((char *)ip) + hdr_bytes);
	__udp_handler(pkt, &r);
	break;
#endif

      default:
	__pktbuf_free(pkt);
	break;
    }
}


/*
 * Send an IP packet.
 *
 * The IP data field should contain pkt->pkt_bytes of data.
 * pkt->[udp|tcp|icmp]_hdr points to the IP data field. Any
 * IP options are assumed to be already in place in the IP
 * options field.
 */
void
__ip_send(pktbuf_t *pkt, int protocol, ip_route_t *dest)
{
    ip_header_t *ip = pkt->ip_hdr;
    int         hdr_bytes;

    /*
     * Figure out header length. The use udp_hdr is
     * somewhat arbitrary, but works because it is
     * a union with other IP protocol headers.
     */
    hdr_bytes = ((char *)pkt->udp_hdr) - ((char *)ip);

    pkt->pkt_bytes += hdr_bytes;

    ip->version = 4;
    ip->hdr_len = hdr_bytes >> 2;
    ip->tos = 0;
    ip->length = pkt->pkt_bytes;
    ip->ident = ip_ident++;
    ip->fragment = 0;
    ip->ttl = 255;
    ip->protocol = protocol;
    ip->checksum = 0;
    memcpy(ip->source, __local_ip_addr, sizeof(ip_addr_t));
    memcpy(ip->destination, dest->ip_addr, sizeof(ip_addr_t));
    ip->checksum = __sum((word *)ip, hdr_bytes, 0);

    __enet_send(pkt, dest->enet_addr, ETH_TYPE_IP);    
}


