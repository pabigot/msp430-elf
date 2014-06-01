/*
 * arp.c -- Polling ARP/RARP for Cygnus BSP
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
#include <string.h>
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "net.h"

static struct {
    int      waiting;
    char     *eth;
    char     *ip;
} arp_req;

/*
 * Handle incoming ARP packets.
 */
void
__arp_handler(pktbuf_t *pkt)
{
    arp_header_t *arp = pkt->arp_hdr;

    /*
     * Only handle ethernet hardware and IP protocol.
     */
    if (arp->hw_type == htons(ARP_HW_ETHER) &&
	arp->protocol == htons(ETH_TYPE_IP)) {
	/*
	 * Handle requests for our ethernet address.
	 */
	if (!memcmp(arp->target_ip, __local_ip_addr, 4)) {
	    if (arp->opcode == htons(ARP_REQUEST)) {
		/* format response. */
		arp->opcode = htons(ARP_REPLY);
		memcpy(arp->target_ip, arp->sender_ip,
		       sizeof(ip_addr_t));
		memcpy(arp->target_enet, arp->sender_enet,
		       sizeof(enet_addr_t));
		memcpy(arp->sender_ip, __local_ip_addr,
		       sizeof(ip_addr_t));
		memcpy(arp->sender_enet, __local_enet_addr,
		       sizeof(enet_addr_t));
		pkt->pkt_bytes = sizeof(arp_header_t);
		__enet_send(pkt, arp->target_enet, ETH_TYPE_ARP);

	    } else if (arp->opcode == htons(ARP_REPLY) && arp_req.waiting) {
		if (!memcmp(arp_req.ip, arp->sender_ip, sizeof(ip_addr_t))) {
                    memcpy(arp_req.eth, arp->sender_enet, sizeof(enet_addr_t));
		    arp_req.waiting = 0;
		}
	    }
	}
    }
    __pktbuf_free(pkt);
}


/* 
 * Find the ethernet address of the machine with the given
 * ip address.
 * Return true and fills in 'eth_addr' if successful, false
 * if unsuccessful.
 */
int
__arp_request(ip_addr_t ip_addr, enet_addr_t eth_addr)
{
    pktbuf_t *pkt;
    arp_header_t *arp;
    unsigned long retry_start;
    enet_addr_t   bcast_addr;
    int           retry;

    /* just fail if can't get a buffer */
    if ((pkt = __pktbuf_alloc(ARP_PKT_SIZE)) == NULL)
	return 0;

    arp = pkt->arp_hdr;
    arp->opcode = htons(ARP_REQUEST);
    arp->hw_type = htons(ARP_HW_ETHER);
    arp->protocol = htons(0x800);
    arp->hw_len = sizeof(enet_addr_t);
    arp->proto_len = sizeof(ip_addr_t);

    memcpy(arp->sender_ip, __local_ip_addr, sizeof(ip_addr_t));
    memcpy(arp->sender_enet, __local_enet_addr, sizeof(enet_addr_t));
    memcpy(arp->target_ip, ip_addr, sizeof(ip_addr_t));

    bcast_addr[0] = 255;
    bcast_addr[1] = 255;
    bcast_addr[2] = 255;
    bcast_addr[3] = 255;
    bcast_addr[4] = 255;
    bcast_addr[5] = 255;

    arp_req.eth = eth_addr;
    arp_req.ip = ip_addr;
    arp_req.waiting = 1;

    retry = 8;
    while (retry-- > 0) {

        /* send the packet */
	pkt->pkt_bytes = sizeof(arp_header_t);
        __enet_send(pkt, bcast_addr, ETH_TYPE_ARP);

        retry_start = MS_TICKS();
	while ((MS_TICKS() - retry_start) < 250) {
	    __enet_poll();
	    if (!arp_req.waiting) {
		__pktbuf_free(pkt);
		return 1;
	    }
	}
    }
    __pktbuf_free(pkt);
    return 0;
}

