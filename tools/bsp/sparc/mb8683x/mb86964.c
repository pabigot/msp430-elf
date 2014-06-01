/*
 * mb86964.c -- Simple driver for Fujitsu MB86964 NIC on MB8683x eval board.
 *
 * Copyright (c) 1999 Cygnus Support
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
#include <string.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "mb86964.h"
#include "mb86940.h"
#include "net.h"
#include "bsplog.h"

enet_addr_t __local_enet_addr;           /* local ethernet address */

#ifndef DEBUG_ENET
#define DEBUG_ENET 0
#endif

#ifndef DEBUG_PKT_STATS
#define DEBUG_PKT_STATS 0
#endif

#if DEBUG_ENET
static void DumpStatus(void);
static void DumpARP(unsigned char *eth_pkt);
static void DumpETH(unsigned char *eth_pkt);
#endif

#if DEBUG_PKT_STATS
static int _pkts_sent;
static int _pkts_rcvd;
static int _pkts_crcerr;
static int _pkts_alignerr;
static int _pkts_shorterr;
static int _pkts_longerr;
static void DumpPktStats(void);
#endif

unsigned short
swab(unsigned short s)
{
    return (unsigned short)(s >> 8) | (s << 8);
}


#define SWAB_REGS 0
#define SWAB_BUF  0
#define SWAB_LEN  1
#define SWAB_BIT  0

#if SWAB_REGS
#define swab_reg(x) swab(x)
#else
#define swab_reg(x) (x)
#endif

#if SWAB_BUF
#define swab_buf(x) swab(x)
#else
#define swab_buf(x) (x)
#endif

#if SWAB_LEN
#define swab_len(x) swab(x)
#else
#define swab_len(x) (x)
#endif

#if SWAB_BIT
#define MB_LB_BIT 1
#else
#define MB_LB_BIT 0
#endif

/* interrupt bit in MB86940 regs */
#define IBIT  (1<<14)

static inline void
clear_irq(void)
{
    unsigned irl;

    irl = mb86940_read(MB86940_IRLATCH);
    if ((irl & 15) == 14) {
	mb86940_write(MB86940_IRQCLR, IBIT);
	mb86940_write(MB86940_IRLATCH, 0x10);
    }
}


/* 
 * Initialize the Ethernet Interface, and this package.  Enable input on
 * both buffers.
 */
static void
mb86964_init(enet_addr_t enet_addr)
{
    memcpy(__local_enet_addr, enet_addr, sizeof(enet_addr_t));

#if DEBUG_ENET
    bsp_printf("mb86964_init: eth[%02x:%02x:%02x:%02x:%02x:%02x]\n",
	       enet_addr[0], enet_addr[1], enet_addr[2],
	       enet_addr[3], enet_addr[4], enet_addr[5]);
#endif

    /*
     * First access must be byte wide.
     */
    mb86964_write_config0(0xd2);

    mb86964_write(&mb_regs->config, swab_reg(0xd220 | MB_LB_BIT));

#if DEBUG_PKT_STATS
    /* accept error packets */
    mb86964_write(&mb_regs->mode, swab_reg(0x062a));
#else
    /* accept multicast through hash table */
    mb86964_write(&mb_regs->mode, swab_reg(0x0602));
#endif

    mb86964_write(&mb_regs->u.rbs0.node_01,
		  swab_reg((__local_enet_addr[0]<<8) | __local_enet_addr[1]));
    mb86964_write(&mb_regs->u.rbs0.node_23,
		  swab_reg((__local_enet_addr[2]<<8) | __local_enet_addr[3]));
    mb86964_write(&mb_regs->u.rbs0.node_45,
		  swab_reg((__local_enet_addr[4]<<8) | __local_enet_addr[5]));

    mb86964_write(&mb_regs->config, swab_reg(0xd228 | MB_LB_BIT));

    /* go to next packet after 16 collisions */
    mb86964_write(&mb_regs->u.rbs2.tx_start, swab_reg(7));

    mb86964_write(&mb_regs->u.rbs2.dma, swab_reg(0x00));

    /* clear xcvr status */
    mb86964_write(&mb_regs->u.rbs2.xcvr, swab_reg(0x01fa));

    mb86964_write(&mb_regs->config, swab_reg(0x5228 | MB_LB_BIT));

    mb86964_write(&mb_regs->ien, 0x00);

#if DEBUG_ENET
    DumpStatus();
#endif
}


static int
mb86964_irq_disable(void *unused)
{
    int was_enabled;

    was_enabled = mb86964_read(&mb_regs->ien) & 0x80;
    mb86964_write(&mb_regs->ien, 0x00);

    return was_enabled;
}


static void
mb86964_irq_enable(void *unused)
{
    mb86964_write(&mb_regs->ien, 0x80);
}


/*
 *  Send a packet out over the ethernet.  The packet is sitting at the
 *  beginning of the transmit buffer.  The routine returns when the
 *  packet has been successfully sent.
 */
static void
mb86964_send(void *unused, const char *buf, int length)
{
    int i;
    const unsigned short *sbuf;

    if (length < ETH_MIN_PKTLEN) 
        length = ETH_MIN_PKTLEN; /* and min. ethernet len */

    memcpy(((eth_header_t *)buf)->source, __local_enet_addr,
	   sizeof(__local_enet_addr));

#if 0
    if (((eth_header_t *)buf)->type == ntohs(ETH_TYPE_ARP))
	DumpARP(buf);
#endif

#if (DEBUG_ENET >= 16)
    BSPLOG(bsp_log("mb86964_send: len[%d]\n", length));
#endif

    /*
     *  Make sure the the transmitter is idle.
     */
    while (swab_reg(mb86964_read(&mb_regs->u.rbs2.tx_start)) & 0x7f00)
	;

#if (DEBUG_ENET >= 16)
    BSPLOG(bsp_log("mb86964_send: tx idle\n"));
#endif

    mb86964_write(&mb_regs->u.rbs2.buffer, swab_len(length));

    length = (length + 1) >> 1;
    sbuf = (const unsigned short *)buf;

#if (DEBUG_ENET >= 16)
    BSPLOG(bsp_log("mb86964_send: buf[0x%x] len[%d]\n", sbuf, length));
#endif

    for (i = 0; i < length; i++)
	mb86964_write(&mb_regs->u.rbs2.buffer, swab_buf(*sbuf++));

    /* send the packet */
    mb86964_write(&mb_regs->u.rbs2.tx_start, swab_reg(0x8103));

#if (DEBUG_ENET >= 16)
    BSPLOG(bsp_log("Waiting for TXDONE\n"));

    /* and wait until it has really been sent. */
    while (!(swab_reg(mb86964_read(&mb_regs->status)) & TX_DONE))
	;
    BSPLOG(bsp_log("done\n"));
#endif
}


/* 
 * Test for the arrival of a packet on the Ethernet interface.  The packet may
 * arrive in either buffer A or buffer B; the location of the packet is
 * returned.  If no packet is returned withing 'timeout' milliseconds,
 * then the routine returns zero.
 * 
 * Note: ignores ethernet errors.  may occasionally return something
 * which was received in error.
 */
int
mb86964_receive(void *unused, char *buf, int maxlen)
{
    int i, pktlen, extra;
    unsigned short stat;
    unsigned short *p, dummy;

#if DEBUG_PKT_STATS
    {
	static int xx = 0;

	if ((++xx % 200000) == 0)
	    DumpPktStats();
    }
#endif
#if (DEBUG_ENET >= 16)
    {
	static int xx = 0;

	if ((++xx % 100000) == 0) {
	    BSPLOG(bsp_log("rcv:\n"));
	    DumpStatus();
	}
    }
#endif

    if (swab_reg(mb86964_read(&mb_regs->mode)) & RX_BUF_EMPTY) {
	mb86964_write(&mb_regs->status, swab_reg(0x80));
	clear_irq();
	if (swab_reg(mb86964_read(&mb_regs->mode)) & RX_BUF_EMPTY)
	    return 0;
    }

    /* reset interrupt flag */
    mb86964_write(&mb_regs->status, swab_reg(0x80));
    clear_irq();

    stat = swab_buf(mb86964_read(&mb_regs->u.rbs2.buffer));
    pktlen = swab_len(mb86964_read(&mb_regs->u.rbs2.buffer));

#if DEBUG_PKT_STATS
    ++_pkts_rcvd;
#endif

#if (DEBUG_ENET >= 16)
    BSPLOG(bsp_log("pkt rx: stat[0x%04x] len[%d]\n", stat, pktlen));
#endif

    if (pktlen <= maxlen)
	extra = 0;
    else {
#if DEBUG_ENET
	BSPLOG(bsp_log("mb86964_receive: excessive pktlen<%d>.\n", pktlen));
#endif
#if (DEBUG_ENET >= 8)
	DumpStatus();
#endif
#if DEBUG_PKT_STATS
	++_pkts_longerr;
#endif
	extra = pktlen - maxlen;
	pktlen = maxlen;
    }

    /* packet data */
    for (i = 0, p = (unsigned short *)buf; i < ((pktlen + 1) >> 1); i++)
	*p++ = swab_buf(mb86964_read(&mb_regs->u.rbs2.buffer));

    /* overflow data */
    for (i = 0; i < ((extra + 1)>>1); i++)
	dummy = swab_buf(mb86964_read(&mb_regs->u.rbs2.buffer));

    if (!extra && (stat & 0x2000) && pktlen >= ETH_MIN_PKTLEN) {
#if (DEBUG_ENET >= 4)
	BSPLOG(bsp_log("got pkt: 0x%x\n", ntohs(((eth_header_t *)buf)->type)));
#endif
#if 0
	if (((eth_header_t *)buf)->type == htons(ETH_TYPE_ARP))
	    DumpARP(buf);
	else if (((eth_header_t *)buf)->type != htons(ETH_TYPE_IP))
	    DumpETH(buf);
	DumpStatus();
#endif
	return pktlen;
    }

#if DEBUG_PKT_STATS
    if (stat & 0x800)
	++_pkts_shorterr;
    if (stat & 0x400)
	++_pkts_alignerr;
    if (stat & 0x200)
	++_pkts_crcerr;
#endif

#if DEBUG_ENET
    BSPLOG(bsp_log("got Bad pkt. extra[%d] stat[%04x] pktlen[%d]\n",
		   extra, stat, pktlen));
    DumpETH(buf);
#endif
#if (DEBUG_ENET >= 8)
    DumpStatus();
#endif
    return 0;
}

extern int gdb_interrupt_check(void);

static int
irq_handler(int irq_nr, void *regs)
{
    BSPLOG(bsp_log("Net irq.\n"));
    if (gdb_interrupt_check()) {
	BSPLOG(bsp_log("Got ^C.\n"));
	return 0;
    }
    BSPLOG(bsp_log("irq done.\n"));
    return 1;
}


static int
mb86964_control(void *unused, int func, ...)
{
    int          retval = 0;
    va_list      ap;
    static bsp_vec_t eth_vec;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	retval = -1;
	break;

      case COMMCTL_GETBAUD:
	retval = -1;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	eth_vec.handler = irq_handler;
	eth_vec.next = NULL;

	bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_INT14,
			BSP_VEC_REPLACE, &eth_vec);
	bsp_enable_irq(BSP_EXC_INT14);

	mb86964_irq_enable(NULL);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	mb86964_irq_disable(NULL);
	bsp_disable_irq(BSP_EXC_INT14);
	bsp_remove_vec(BSP_VEC_EXCEPTION, BSP_EXC_INT14, &eth_vec);
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = mb86964_irq_disable(NULL);
	break;

      case COMMCTL_IRQ_ENABLE:
	mb86964_irq_enable(NULL);
	break;

      default:
	retval = -1;
	break;
    }

    va_end(ap);
    return retval;
}

#if DEBUG_ENET
static void
DumpStatus(void)
{
    unsigned short stat, mode, start, config, xcvr;

    stat = swab_reg(mb86964_read(&mb_regs->status));
    mode = swab_reg(mb86964_read(&mb_regs->mode));
    config = swab_reg(mb86964_read(&mb_regs->config));
    start = swab_reg(mb86964_read(&mb_regs->u.rbs2.tx_start));
    xcvr = swab_reg(mb86964_read(&mb_regs->u.rbs2.xcvr));

    BSPLOG(bsp_log("stat[0x%x] mode[0x%x]\n", stat, mode));

    BSPLOG(bsp_log("tx_start[0x%x] config[0x%x] xcvr[0x%x]\n",
		start, config, xcvr));
}

const static char *arp_op[] = {
    "invalid",
    "ARP request",
    "ARP reply",
    "RARP request",
    "RARP reply"
};

static void
DumpARP(unsigned char *eth_pkt)
{
    arp_header_t *ahdr = (arp_header_t *)(eth_pkt + sizeof(eth_header_t));

    BSPLOG(bsp_log("%s info:\n",arp_op[ahdr->opcode]));
    BSPLOG(bsp_log("  hw_type[%d]  protocol[%d]  hwlen[%d]  protolen[%d]\n",
	       ntohs(ahdr->hw_type), ntohs(ahdr->protocol),
	       ahdr->hw_len, ahdr->proto_len));
    BSPLOG(bsp_log("  sender_enet: %02x.%02x.%02x.%02x.%02x.%02x\n",
	       ahdr->sender_enet[0], ahdr->sender_enet[1],
	       ahdr->sender_enet[2], ahdr->sender_enet[3],
	       ahdr->sender_enet[4], ahdr->sender_enet[5]));
    BSPLOG(bsp_log("  sender_ip:  %d.%d.%d.%d\n",
	       ahdr->sender_ip[0], ahdr->sender_ip[1],
	       ahdr->sender_ip[2], ahdr->sender_ip[3]));
    BSPLOG(bsp_log("  target_eth: %02x.%02x.%02x.%02x.%02x.%02x\n",
	       ahdr->target_enet[0], ahdr->target_enet[1],
	       ahdr->target_enet[2], ahdr->target_enet[3],
	       ahdr->target_enet[4], ahdr->target_enet[5]));
    BSPLOG(bsp_log("  target_ip:  %d.%d.%d.%d\n",
	       ahdr->target_ip[0], ahdr->target_ip[1],
	       ahdr->target_ip[2], ahdr->target_ip[3]));
}

static void
DumpETH(unsigned char *eth_pkt)
{
    eth_header_t *ehdr = (eth_header_t *)eth_pkt;

    BSPLOG(bsp_log("dest: %02x.%02x.%02x.%02x.%02x.%02x\n",
	       ehdr->destination[0], ehdr->destination[1],
	       ehdr->destination[2], ehdr->destination[3],
	       ehdr->destination[4], ehdr->destination[5]));
    BSPLOG(bsp_log("src:  %02x.%02x.%02x.%02x.%02x.%02x\n",
	       ehdr->source[0], ehdr->source[1],
	       ehdr->source[2], ehdr->source[3],
	       ehdr->source[4], ehdr->source[5]));
    BSPLOG(bsp_log("type: %x\n", ntohs(ehdr->type)));
}
#endif

#if DEBUG_PKT_STATS
static void
DumpPktStats(void)
{
    static int _last_sent;
    static int _last_rcvd;
    static int _last_crcerr;
    static int _last_alignerr;
    static int _last_shorterr;
    static int _last_longerr;
    int  msec;

    BSPLOG(bsp_log("Eth Stats:\n"));
    BSPLOG(bsp_log("  sent:     %d (%d)\n", _pkts_sent, _pkts_sent - _last_sent));
    _last_sent = _pkts_sent;
    
    BSPLOG(bsp_log("  rcvd:     %d (%d)\n", _pkts_rcvd, _pkts_rcvd - _last_rcvd));
    _last_rcvd = _pkts_rcvd;
    
    BSPLOG(bsp_log("  crcerr:   %d (%d)\n", _pkts_crcerr, _pkts_crcerr - _last_crcerr));
    _last_crcerr = _pkts_crcerr;
    
    BSPLOG(bsp_log("  alignerr: %d (%d)\n", _pkts_alignerr, _pkts_alignerr - _last_alignerr));
    _last_alignerr = _pkts_alignerr;
    
    BSPLOG(bsp_log("  short:    %d (%d)\n", _pkts_shorterr, _pkts_shorterr - _last_shorterr));
    _last_shorterr = _pkts_shorterr;
    
    BSPLOG(bsp_log("  long:     %d (%d)\n", _pkts_longerr, _pkts_longerr - _last_longerr));
    _last_longerr = _pkts_longerr;

    BSPLOG(bsp_log("  txtime: %d bytes: %d\n", _pkts_txtime, _pkts_txbytes));
    BSPLOG(bsp_log("  rxtime: %d bytes: %d\n\n", _pkts_rxtime, _pkts_rxbytes));
}
#endif



static void
dummy_putc(void *unused, char ch)
{
}

static int
dummy_getc(void *unused)
{
    return -1;
}

static struct bsp_comm_procs mb86964_procs = {
    NULL,
    mb86964_send, mb86964_receive,
    dummy_putc, dummy_getc,
    mb86964_control
};

struct bsp_comm_procs *
__enet_link_init(enet_addr_t enet_addr)
{
    mb86964_init(enet_addr);
    
    return &mb86964_procs;
}

