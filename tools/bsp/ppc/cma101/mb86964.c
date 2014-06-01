/* 
 *
 * Ethernet Driver.
 * A very simple set of ethernet driver primitives.
 *
 */
#include <string.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "mb86964.h"
#include "net.h"

enet_addr_t __local_enet_addr;           /* local ethernet address */

#ifndef DEBUG_ENET
#define DEBUG_ENET 0
#endif

#if DEBUG_ENET
static void DumpStatus(void);
static void DumpARP(unsigned char *eth_pkt);
#endif

/* 
 * Initialize the Ethernet Interface, and this package.  Enable input on
 * both buffers.
 */
static void
mb86964_init(enet_addr_t enet_addr)
{
    int  was_cached;

    memcpy(__local_enet_addr, enet_addr, sizeof(enet_addr_t));

    was_cached = __dcache_disable();

    mb_regs->config0 = 0xf6;
    mb_regs->config1 = 0x20;
    mb_regs->rx_mode = 2; /* accept multicast through hash table */

    mb_regs->u.rbs0.node_0 = __local_enet_addr[0];
    mb_regs->u.rbs0.node_1 = __local_enet_addr[1];
    mb_regs->u.rbs0.node_2 = __local_enet_addr[2];
    mb_regs->u.rbs0.node_3 = __local_enet_addr[3];
    mb_regs->u.rbs0.node_4 = __local_enet_addr[4];
    mb_regs->u.rbs0.node_5 = __local_enet_addr[5];
    mb_regs->config1 = 0x28;

    /* go to next packet after 16 collisions */
    mb_regs->u.rbs2.collision = 7;
    mb_regs->u.rbs2.tx_start  = 0;
    mb_regs->config0 = 0x76;

    if (was_cached)
	__dcache_enable();
}

static int
mb86964_irq_disable(void *unused)
{
    int was_enabled, was_cached;

    was_cached = __dcache_disable();
    was_enabled = mb_regs->rx_ien & 0x80;
    mb_regs->rx_ien = 0x00;
    if (was_cached)
	__dcache_enable();

    return was_enabled;
}


static void
mb86964_irq_enable(void *unused)
{
    int was_cached;
    was_cached = __dcache_disable();
    mb_regs->rx_ien = 0x80;
    if (was_cached)
	__dcache_enable();
}



/*
 *  Send a packet out over the ethernet.  The packet is sitting at the
 *  beginning of the transmit buffer.  The routine returns when the
 *  packet has been successfully sent.
 */
static void
mb86964_send(void *unused, const char *buf, int length)
{
    int i, was_cached;

    if (length < ETH_MIN_PKTLEN) 
        length = ETH_MIN_PKTLEN; /* and min. ethernet len */

    memcpy(((eth_header_t *)buf)->source, __local_enet_addr,
	   sizeof(__local_enet_addr));

    __dcache_flush((char *)buf, length);
    was_cached = __dcache_disable();

#if DEBUG_ENET
    if (((eth_header_t *)buf)->type == ntohs(ETH_TYPE_ARP))
	DumpARP(buf);
#endif

    /*
     *  Make sure there's at least one buffer available.
     *  There's 2 buffers, so wait till only 0 or 1 are 
     *  in use.
     */
    while (mb_regs->u.rbs2.tx_start & 0x7f)
	;

    mb_regs->u.rbs2.buffer = length & 0xff;
    mb_regs->u.rbs2.buffer = (length >> 8) & 0xff;

    for (i = 0; i < length; i++)
	mb_regs->u.rbs2.buffer = *buf++;

    /* send the packet */
    mb_regs->u.rbs2.tx_start = 0x81;

#if 0
    /* and wait until it has really been sent. */
    while (!(mb_regs->tx_status & TX_DONE))
	;
#endif

    if (was_cached)
	__dcache_enable();
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
    int i, pktlen, extra, was_cached;
    unsigned char *p, dummy, stat;

    __dcache_flush(buf, maxlen);
    was_cached = __dcache_disable();

    if (mb_regs->rx_mode & RX_BUF_EMPTY) {
	mb_regs->rx_status = 0x80;
	if (mb_regs->rx_mode & RX_BUF_EMPTY) {
	    if (was_cached)
		__dcache_enable();
	    return 0;
	}
    }

    mb_regs->rx_status = 0x80;

    stat = mb_regs->u.rbs2.buffer;            /* status   */
    dummy = mb_regs->u.rbs2.buffer;           /* reserved */
    pktlen = mb_regs->u.rbs2.buffer;          /* length LSB */
    pktlen |= (mb_regs->u.rbs2.buffer << 8);  /* length MSB */

    if (pktlen <= maxlen)
	extra = 0;
    else {
	bsp_printf("mb86964_receive: excessive pktlen<%d>.\n", pktlen);
#if DEBUG_ENET
	DumpStatus();
#endif
	extra = pktlen - maxlen;
	pktlen = maxlen;
    }

    for (i = 0, p = buf; i < pktlen; i++)
	*p++ = mb_regs->u.rbs2.buffer;  /* packet data */

    for (i = 0; i < extra; i++)
	dummy = mb_regs->u.rbs2.buffer;  /* overflow data */

    if (was_cached)
	__dcache_enable();

    if (!extra && (stat & 0x20) && pktlen >= ETH_MIN_PKTLEN) {
#if DEBUG_ENET
	bsp_printf("got pkt: 0x%x\n", ntohs(((enet_header_t *)buf)->type));
	if (((enet_header_t *)buf)->type == htons(ETH_TYPE_ARP))
	    DumpARP(buf);
#endif
	return pktlen;
    }

#if DEBUG_ENET
    bsp_printf("got Bad pkt.\n");
    DumpStatus();
#endif

    return 0;
}

extern int gdb_interrupt_check(void);
static int
irq_handler(int irq_nr, void *regs)
{
    if (gdb_interrupt_check())
	return 0;
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

	bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW2,
			BSP_VEC_REPLACE, &eth_vec);
	bsp_enable_irq(BSP_IRQ_HW2);
	mb86964_irq_enable(NULL);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	mb86964_irq_disable(NULL);
	bsp_disable_irq(BSP_IRQ_HW2);
	bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW2, &eth_vec);
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
    unsigned char tx_stat, rx_stat, tx_mode, rx_mode, tx_start,
                  config1, xcvr_mode, xcvr_stat;
    int  was_cached;

    was_cached = __dcache_disable();

    tx_stat = mb_regs->tx_status;
    rx_stat = mb_regs->rx_status;
    tx_mode = mb_regs->tx_mode;
    rx_mode = mb_regs->rx_mode;
    config1 = mb_regs->config1;
    tx_start = mb_regs->u.rbs2.tx_start;
    xcvr_mode = mb_regs->u.rbs2.dma_burst;
    xcvr_stat = mb_regs->u.rbs2.xcvr_status;

    if (was_cached)
	__dcache_enable();

    bsp_printf("tx_stat[0x%x] rx_stat[0x%x] tx_mode[0x%x] rx_mode[0x%x]\n",
		tx_stat, rx_stat, tx_mode, rx_mode);

    bsp_printf("tx_start[0x%x] config1[0x%x] xcvr_mode[0x%x] xcvr_stat[0x%x]\n",
		tx_start, config1, xcvr_mode, xcvr_stat);
}

static char *arp_op[] = {
    "invalid",
    "ARP request",
    "ARP reply",
    "RARP request",
    "RARP reply"
};

static void
DumpARP(unsigned char *eth_pkt)
{
    arp_header_t *ahdr = (arp_header_t *)(eth_pkt + sizeof(enet_header_t));

    bsp_printf("%s info:\n",arp_op[ahdr->opcode]);
    bsp_printf("  hw_type[%d]  protocol[%d]  hwlen[%d]  protolen[%d]\n",
	       ntohs(ahdr->hw_type), ntohs(ahdr->protocol),
	       ahdr->hw_len, ahdr->proto_len);
    bsp_printf("  sender_enet: %02x.%02x.%02x.%02x.%02x.%02x\n",
	       ahdr->sender_enet[0], ahdr->sender_enet[1],
	       ahdr->sender_enet[2], ahdr->sender_enet[3],
	       ahdr->sender_enet[4], ahdr->sender_enet[5]);
    bsp_printf("  sender_ip:  %d.%d.%d.%d\n",
	       ahdr->sender_ip[0], ahdr->sender_ip[1],
	       ahdr->sender_ip[2], ahdr->sender_ip[3]);
    bsp_printf("  target_eth: %02x.%02x.%02x.%02x.%02x.%02x\n",
	       ahdr->target_enet[0], ahdr->target_enet[1],
	       ahdr->target_enet[2], ahdr->target_enet[3],
	       ahdr->target_enet[4], ahdr->target_enet[5]);
    bsp_printf("  target_ip:  %d.%d.%d.%d\n",
	       ahdr->target_ip[0], ahdr->target_ip[1],
	       ahdr->target_ip[2], ahdr->target_ip[3]);
}
#endif


static void
dummy_putc(void *unused, char ch)
{
    bsp_printf("putc not supported for an enet link driver\n");
}

static int
dummy_getc(void *unused)
{
    bsp_printf("getc not supported for an enet link driver\n");
    return 0;
}


struct bsp_comm_procs mb86964_procs = {
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

