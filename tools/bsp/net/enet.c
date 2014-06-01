/*
 * enet.c -- Polling ethernet link-layer for Cygnus BSP
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
#include "bsplog.h"

static struct bsp_comm_procs *enet_procs;

#define ENET_STATS 1

#if ENET_STATS
static int num_ip = 0;
static int num_arp = 0;
#if NET_SUPPORT_RARP
static int num_rarp = 0;
#endif
static int num_received = 0;
static int num_transmitted = 0;
#endif

/*
 * Install procs for link driver.
 */
void
_bsp_install_enet_driver(struct bsp_comm_procs *procs)
{
    enet_procs = procs;
}

/*
 * Non-blocking poll of ethernet link. Processes at most
 * one packet.
 */
void
__enet_poll(void)
{
    pktbuf_t *pkt;

    /*
     * Try to get a free pktbuf and return if none
     * are available.
     */
    if ((pkt = __pktbuf_alloc(ETH_MAX_PKTLEN)) == NULL) {
	BSPLOG(bsp_log("__enet_poll: no more buffers.\n"));
	BSPLOG(bsp_log_dump());
	BSPLOG(while(1));
	return;
    }

    if ((pkt->pkt_bytes = (*enet_procs->__read)(enet_procs->ch_data,
						(char *)pkt->eth_hdr,
						ETH_MAX_PKTLEN)) > 0) {
#if ENET_STATS
	++num_received;
#endif
	switch (pkt->eth_hdr->type) {

	  case htons(ETH_TYPE_IP):
#if ENET_STATS
	    ++num_ip;
#endif
	    pkt->ip_hdr = (ip_header_t *)(pkt->eth_hdr + 1);
	    pkt->pkt_bytes -= sizeof(eth_header_t);
	    __ip_handler(pkt, pkt->eth_hdr->source);
	    break;

	  case htons(ETH_TYPE_ARP):
#if ENET_STATS
	    ++num_arp;
#endif
	    pkt->arp_hdr = (arp_header_t *)(pkt->eth_hdr + 1);
	    pkt->pkt_bytes -= sizeof(eth_header_t);
	    __arp_handler(pkt);
	    break;

#if NET_SUPPORT_RARP
	  case htons(ETH_TYPE_RARP):
#if ENET_STATS
	    ++num_rarp;
#endif
	    pkt->arp_hdr = (arp_header_t *)(pkt->eth_hdr + 1);
	    pkt->pkt_bytes -= sizeof(eth_header_t);
	    __rarp_handler(pkt);
	    break;
#endif

	  default:
	    __pktbuf_free(pkt);
	    break;
	}
    }
    else
	__pktbuf_free(pkt);
}



/*
 * Send an ethernet packet.
 */
void
__enet_send(pktbuf_t *pkt, enet_addr_t dest, int eth_type)
{
    /*
     * Add destination address here. Link driver adds ethernet source
     * address.
     */
    memmove(pkt->eth_hdr->destination, dest, sizeof(enet_addr_t));
    pkt->eth_hdr->type = htons(eth_type);

    pkt->pkt_bytes += sizeof(eth_header_t);

    (*enet_procs->__write)(enet_procs->ch_data,
			   (const char *)pkt->eth_hdr,
			   pkt->pkt_bytes);
#if ENET_STATS
    ++num_transmitted;
#endif
}



