/*
 * pktbuf.c -- Simple management of packet buffers.
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

#define MAX_PKTBUF 4

#define BUFF_STATS 1

#if BUFF_STATS
int max_alloc = 0;
int num_alloc = 0;
int num_free  = 0;
#endif

static pktbuf_t  pktbuf_list[MAX_PKTBUF];
static word      bufdata[MAX_PKTBUF][ETH_MAX_PKTLEN/2 + 1];
static pktbuf_t *free_list;


/*
 * Initialize the free list.
 */
void
__pktbuf_init(void)
{
    int  i;
    word *p;

    for (i = 0; i < MAX_PKTBUF; i++) {
	p = bufdata[i];
	if ((((unsigned long)p) & 2) == 0)
	    ++p;
	pktbuf_list[i].buf = p;
	pktbuf_list[i].bufsize = ETH_MAX_PKTLEN;
	pktbuf_list[i].next = free_list;
	free_list = &pktbuf_list[i];
    }
}


/*
 * simple pktbuf allocation
 */
pktbuf_t *
__pktbuf_alloc(int nbytes)
{
    pktbuf_t *p = free_list;

    if (p) {
	free_list = p->next;
	p->eth_hdr = (eth_header_t *)p->buf;
	p->ip_hdr  = (ip_header_t *)(p->eth_hdr + 1);
	p->tcp_hdr = (tcp_header_t *)(p->ip_hdr + 1);
	p->pkt_bytes = 0;
#if BUFF_STATS
	++num_alloc;
	if ((num_alloc - num_free) > max_alloc)
	    max_alloc = num_alloc - num_free;
#endif
    }
    return p;
}


/*
 * free a pktbuf.
 */
void
__pktbuf_free(pktbuf_t *pkt)
{
#if BUFF_STATS
    --num_alloc;
#endif
#ifdef BSP_LOG
    {
	int i;
	word *p;

	for (i = 0; i < MAX_PKTBUF; i++) {
	    p = bufdata[i];
	    if ((((unsigned long)p) & 2) == 0)
		++p;
	    if (p == (word *)pkt)
		break;
	}
	if (i < MAX_PKTBUF) {
	    BSPLOG(bsp_log("__pktbuf_free: bad pkt[%x].\n", pkt));
	    BSPLOG(bsp_log_dump());
	    BSPLOG(while(1));
	}
    }
#endif
    pkt->next = free_list;
    free_list = pkt;
}


