/*
 * icmp.c -- Limited ICMP (ping only) for Cygnus BSP
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
#include "bsp_if.h"
#include "net.h"
#include "bsplog.h"

/*
 * Handle ICMP packets.
 */
void
__icmp_handler(pktbuf_t *pkt, ip_route_t *r)
{
    if (pkt->icmp_hdr->type == ICMP_TYPE_ECHOREQUEST &&
	pkt->icmp_hdr->code == 0 &&
	__sum((word *)pkt->icmp_hdr, pkt->pkt_bytes, 0) == 0) {

	pkt->icmp_hdr->type = ICMP_TYPE_ECHOREPLY;
	pkt->icmp_hdr->checksum = 0;
	pkt->icmp_hdr->checksum = __sum((word *)pkt->icmp_hdr,
					pkt->pkt_bytes, 0);
	
	BSPLOG(bsp_log("icmp: seq<%d>\n", pkt->icmp_hdr->seqnum));

	__ip_send(pkt, IP_PROTO_ICMP, r);
    }
    __pktbuf_free(pkt);
}


