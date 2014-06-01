/*
 * ne2k.c -- Driver for NE2000 (83905) Ethernet Controller
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
#include "net.h"
#include "bsplog.h"
#include "mb86941.h"
#include "plx9080.h"

#define PLX_PCIMEM_BASE 0x56800000

#define NE2_IRQ    BSP_EXC_INT2
#define NE2_BASE   PLX_PCIMEM_BASE

/* The command register (for all pages) */
#define NE2_CMD		(NE2_BASE+0x00)
#define NE2_DATA	(NE2_BASE+0x10)
#define NE2_RESET	(NE2_BASE+0x1f)

/* Page 0 register offsets. */
#define PG0_CLDA0	(NE2_BASE+0x01)
#define PG0_PSTART	(NE2_BASE+0x01)
#define PG0_CLDA1	(NE2_BASE+0x02)
#define PG0_PSTOP	(NE2_BASE+0x02)
#define PG0_BOUNDARY	(NE2_BASE+0x03)
#define PG0_TSR		(NE2_BASE+0x04)
#define PG0_TPSR	(NE2_BASE+0x04)
#define PG0_NCR		(NE2_BASE+0x05)
#define PG0_TBCR0	(NE2_BASE+0x05)
#define PG0_FIFO	(NE2_BASE+0x06)
#define PG0_TBCR1	(NE2_BASE+0x06)
#define PG0_ISR		(NE2_BASE+0x07)
#define PG0_CRDA0	(NE2_BASE+0x08)
#define PG0_RSAR0	(NE2_BASE+0x08)
#define PG0_CRDA1	(NE2_BASE+0x09)
#define PG0_RSAR1	(NE2_BASE+0x09)
#define PG0_RBCR0	(NE2_BASE+0x0a)
#define PG0_RBCR1	(NE2_BASE+0x0b)
#define PG0_RSR		(NE2_BASE+0x0c)
#define PG0_RCR		(NE2_BASE+0x0c)
#define PG0_CNTR0	(NE2_BASE+0x0d)
#define PG0_TCR		(NE2_BASE+0x0d)
#define PG0_CNTR1	(NE2_BASE+0x0e)
#define PG0_DCR		(NE2_BASE+0x0e)
#define PG0_CNTR2	(NE2_BASE+0x0f)
#define PG0_IMR		(NE2_BASE+0x0f)

#define NE2_WRITE(val,addr)    (*(volatile unsigned char *)(addr)=(val))
#define NE2_READ(addr)         (*(volatile unsigned char *)(addr))
#define NE2_WRITE16(val,addr)  (*(volatile unsigned short *)(addr)=(val))
#define NE2_READ16(addr)       (*(volatile unsigned short *)(addr))

#define NE2_DELAY() { int x; for (x=0; x<200000; x++); }

/*  Command register */
#define CMD_STOP	0x01
#define CMD_START	0x02
#define CMD_TXSTART	0x04
#define CMD_RREAD	0x08
#define CMD_RWRITE	0x10
#define CMD_NODMA	0x20
#define CMD_PAGE0	0x00
#define CMD_PAGE1	0x40
#define CMD_PAGE2	0x80
#define CMD_PAGE3	0xC0

/* Page0 ISR register */
#define ISR_RX		0x01
#define ISR_TX		0x02
#define ISR_RXERR	0x04
#define ISR_TXERR	0x08
#define ISR_OVFLO	0x10
#define ISR_CNT		0x20
#define ISR_RDC		0x40
#define ISR_RESET	0x80
#define ISR_ALL		(ISR_CNT|ISR_OVFLO|ISR_TXERR|ISR_RXERR|ISR_TX|ISR_RX)

/* Page0 DCR register */
#define DCR_WORD	0x01
#define DCR_LE		0x00
#define DCR_BE		0x02
#define DCR_DMA16	0x00
#define DCR_DMA32	0x04
#define DCR_LOOP_ON	0x00
#define DCR_LOOP_OFF	0x08
#define DCR_ARM		0x10
#define DCR_FIFO2	0x00
#define DCR_FIFO4	0x20
#define DCR_FIFO8	0x40
#define DCR_FIFO12	0x60

/* RCR register */
#define RCR_SEP		0x01
#define RCR_AR		0x02
#define RCR_AB		0x04
#define RCR_AM		0x08
#define RCR_PRO		0x10
#define RCR_MON		0x20

/* TCR register */
#define TCR_CRC		0x01
#define TCR_LB0		0x02
#define TCR_LB1		0x04
#define TCR_LOOP_OFF    0x00
#define TCR_LOOP_NIC    TCR_LB0
#define TCR_LOOP_ENDIC  TCR_LB1
#define TCR_LOOP_EXT    (TCR_LB1|TCR_LB0)
#define TCR_ATD		0x08
#define TCR_OFST	0x10

/* Page 1 register offsets. */
#define PG1_PAR0	(NE2_BASE+1)
#define PG1_PAR1	(NE2_BASE+2)
#define PG1_PAR2	(NE2_BASE+3)
#define PG1_PAR3	(NE2_BASE+4)
#define PG1_PAR4	(NE2_BASE+5)
#define PG1_PAR5	(NE2_BASE+6)
#define PG1_CURR	(NE2_BASE+7)
#define PG1_MAR0	(NE2_BASE+8)
#define PG1_MAR1	(NE2_BASE+9)
#define PG1_MAR2	(NE2_BASE+10)
#define PG1_MAR3	(NE2_BASE+11)
#define PG1_MAR4	(NE2_BASE+12)
#define PG1_MAR5	(NE2_BASE+13)
#define PG1_MAR6	(NE2_BASE+14)
#define PG1_MAR7	(NE2_BASE+15)


#define TXBUF_START  0x40
#define BUF_END      0x80

/* 6 buffers == 1536 bytes */
#define TX_PAGES_PER_BUF 6

#define RXBUF_START  (TXBUF_START+(TX_PAGES_PER_BUF * 2))

#define RXBUF_PREV(n) ((((n)-1) < RXBUF_START)?(BUF_END-1):((n)-1))
#define RXBUF_NEXT(n) ((((n)+1) == BUF_END)?(RXBUF_START):((n)+1))

/* these are the supported interrupts */
#define IMR_VAL (ISR_RX|ISR_RXERR|ISR_OVFLO)

#define SWAB(n) ((((n)>>8)&0xff)|(((n)<<8)&0xff00))


static unsigned char next_page;

enet_addr_t __local_enet_addr;           /* local ethernet address */


typedef struct {
    unsigned char  status;
    unsigned char  next;
    unsigned short count;
} ne2_header_t;



/* wait for DMA interrupt, then ack it. */
static inline void
wait_for_dma(void)
{
    while ((NE2_READ(PG0_ISR) & ISR_RDC) == 0)
	;
    NE2_WRITE(ISR_RDC, PG0_ISR);
}


static void
dma_read(unsigned short src, unsigned short *buf, int nbytes)
{
    int i;

    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_START, NE2_CMD);
    NE2_WRITE(nbytes & 0xff, PG0_RBCR0);
    NE2_WRITE((nbytes >> 8) & 0xff, PG0_RBCR1);
    NE2_WRITE(src & 0xff, PG0_RSAR0);
    NE2_WRITE((src >> 8) & 0xff, PG0_RSAR1);
    NE2_WRITE(CMD_RREAD|CMD_PAGE0|CMD_START, NE2_CMD);

    for (i = 0; i < nbytes; i += 2)
	*buf++ = NE2_READ16(NE2_DATA);

    wait_for_dma();
}


static void
dma_write(unsigned short *buf, unsigned short dest, int nbytes)
{
    int i;

    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_START, NE2_CMD);
    NE2_WRITE(nbytes & 0xff, PG0_RBCR0);
    NE2_WRITE((nbytes >> 8) & 0xff, PG0_RBCR1);
    NE2_WRITE(dest & 0xff, PG0_RSAR0);
    NE2_WRITE((dest >> 8) & 0xff, PG0_RSAR1);
    NE2_WRITE(CMD_RWRITE|CMD_PAGE0|CMD_START, NE2_CMD);

    for (i = 0; i < nbytes; i += 2) {
	NE2_WRITE16(*buf, NE2_DATA);
	++buf;
    }

    wait_for_dma();
}


static int
ne2_init(void)
{
    unsigned short prom[16];
    int i;

#if 0
    {
	unsigned long u;

	i = NE2_READ(NE2_RESET);
	u = _bsp_ms_ticks() + 100;
	while (_bsp_ms_ticks() < u)
	    ;
	NE2_WRITE(i, NE2_RESET);

	i = 0;
	while ((NE2_READ(PG0_ISR) & ISR_RESET) == 0)
	    if (++i > 0x200000) {
		bsp_printf("bad reset\n");
		return;
	    }
    }
#endif

    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_STOP, NE2_CMD);
    NE2_WRITE(DCR_FIFO8|DCR_LOOP_OFF|DCR_LE|DCR_DMA16|DCR_WORD, PG0_DCR);
    NE2_WRITE(0, PG0_RBCR0);
    NE2_WRITE(0, PG0_RBCR1);
    NE2_WRITE(0, PG0_IMR);
    NE2_WRITE(ISR_ALL, PG0_ISR);
    NE2_WRITE(RCR_MON, PG0_RCR);
    NE2_WRITE(TCR_LOOP_NIC, PG0_TCR);
    NE2_WRITE(32, PG0_RBCR0);
    NE2_WRITE(0,  PG0_RBCR1);
    NE2_WRITE(0, PG0_RSAR0);
    NE2_WRITE(0, PG0_RSAR1);
    NE2_WRITE(CMD_RREAD|CMD_START, NE2_CMD);

    for (i = 0; i < 16; i++)
	prom[i] = NE2_READ16(NE2_DATA);

    /*
     * This will test if we're running on a simulator which
     * doesn't simulate ethernet hw. On real hardware, this
     * will be a non-zero signature.
     */
    if (prom[15] == 0)
	return 0;

    for (i = 0; i < 6; i++)
	__local_enet_addr[i] = (prom[i] >> 8) & 0xff;

    wait_for_dma();

#if 1
    bsp_printf("eth addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       __local_enet_addr[0], __local_enet_addr[1],
	       __local_enet_addr[2], __local_enet_addr[3],
	       __local_enet_addr[4], __local_enet_addr[5]);
#endif

    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_STOP, NE2_CMD);
    NE2_WRITE(DCR_FIFO8|DCR_LOOP_OFF|DCR_LE|DCR_DMA16|DCR_WORD, PG0_DCR);
    NE2_WRITE(0, PG0_RBCR0);
    NE2_WRITE(0, PG0_RBCR1);
    NE2_WRITE(TXBUF_START, PG0_TPSR);
    NE2_WRITE(RXBUF_START, PG0_PSTART);
    NE2_WRITE(RXBUF_START, PG0_BOUNDARY);
    NE2_WRITE(BUF_END, PG0_PSTOP);

    next_page = RXBUF_START + 1;

    NE2_WRITE(ISR_ALL, PG0_ISR);
    NE2_WRITE(0, PG0_IMR);
    
    /* copy the ethernet address */
    NE2_WRITE(CMD_NODMA|CMD_PAGE1|CMD_STOP, NE2_CMD);
    for (i = 0; i < 6; i++)
	NE2_WRITE(__local_enet_addr[i], PG1_PAR0+i);
    NE2_WRITE(next_page, PG1_CURR);
    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_STOP, NE2_CMD);

    NE2_WRITE(ISR_ALL, PG0_ISR);

    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_START, NE2_CMD);
    NE2_WRITE(TCR_LOOP_OFF, PG0_TCR);
    NE2_WRITE(RCR_AB, PG0_RCR);

    return 1;
}


/*
 *  Send a packet out over the ethernet.  The packet is sitting at the
 *  beginning of the transmit buffer.
 */
static void
ne2_send(void *unused, const char *buf, int length)
{
    unsigned char tx_page;

    if (length < ETH_MIN_PKTLEN) 
        length = ETH_MIN_PKTLEN; /* and min. ethernet len */

    tx_page = TXBUF_START;

    memcpy(((eth_header_t *)buf)->source, __local_enet_addr,
	   sizeof(__local_enet_addr));

#if (DEBUG_ENET >= 16)
    BSPLOG(bsp_log("ne2_send: len[%d]\n", length));
#endif

    NE2_WRITE(ISR_RDC, PG0_ISR);

    /* transfer local buffer to NIC ring buffer */
    dma_write((unsigned short *)buf, tx_page << 8, length);

#if 0
    /* wait for previous tx to complete */
    while (NE2_READ(NE2_CMD) & CMD_TXSTART)
	;
#endif

    NE2_WRITE(tx_page, PG0_TPSR);
    NE2_WRITE(length & 0xff, PG0_TBCR0);
    NE2_WRITE(length >> 8, PG0_TBCR1);
    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_TXSTART|CMD_START, NE2_CMD);

    /* wait for tx to complete */
    while ((NE2_READ(PG0_ISR) & (ISR_TX|ISR_TXERR)) == 0)
	;
    NE2_WRITE(ISR_TX|ISR_TXERR, PG0_ISR);
}


static int
ne2_read_pkt(char *buf, int length)
{
    ne2_header_t hdr;

    /* read header */
    dma_read(next_page << 8, (unsigned short *)&hdr, 4);

    hdr.count = SWAB(hdr.count);

#if 0
    bsp_printf("ne2_read_pkt: status[%02x] next[%02x] count[%d]\n",
	       hdr.status, hdr.next, hdr.count);
#endif

    /* read packet */
    dma_read((next_page << 8) + 4, (unsigned short *)buf, hdr.count);

    next_page = hdr.next;
    NE2_WRITE(RXBUF_PREV(next_page), PG0_BOUNDARY);

    return hdr.count;
}


static int
ne2_overflow(char *buf, int length)
{
    int cmd, resend = 0;
    unsigned long ticks;

#if 0
    bsp_printf("rx overflow\n");
#endif

    /* 1. Read and save command reg. */
    cmd = NE2_READ(NE2_CMD);

    /* 2. Issue STOP */
    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_STOP, NE2_CMD);

    /* 3. Wait 2-3ms for possible transmission to complete */
    ticks = _bsp_ms_ticks() + 3;
    while (_bsp_ms_ticks() < ticks)
	;
    
    /* 4. Clear remote byte count */
    NE2_WRITE(0, PG0_RBCR0);
    NE2_WRITE(0, PG0_RBCR1);

    /* 5. Determine if we need a resend */
    if (cmd & CMD_TXSTART) {
	if ((NE2_READ(PG0_ISR) & (ISR_TX|ISR_TXERR)) == 0)
	    resend = 1;
    }
    
    /* 6. Go to loopback mode */
    NE2_WRITE(TCR_LOOP_NIC, PG0_TCR);

    /* 7. Issue START */
    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_START, NE2_CMD);

    /* 8. Remove a packet */
    length = ne2_read_pkt(buf, length);

    /* 9. Reset overflow ISR bit */
    NE2_WRITE(ISR_OVFLO, PG0_ISR);

    /* 10. Leave loopback mode */
    NE2_WRITE(TCR_LOOP_OFF, PG0_TCR);

    /* 11. Resend, if necessary */
    if (resend)
	NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_TXSTART|CMD_START, NE2_CMD);

    return length;
}


/*
 *  Poll for a packet and if one is available, get it.
 */
static int
ne2_receive(void *unused, char *buf, int length)
{
    int istat, curr_page;

    istat = NE2_READ(PG0_ISR);

    NE2_WRITE(CMD_NODMA|CMD_PAGE1|CMD_START, NE2_CMD);
    curr_page = NE2_READ(PG1_CURR);
    NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_START, NE2_CMD);

#if 0
    bsp_printf("ne2_receive: next[%02x] curr[%02x]\n",
	       next_page, curr_page);
#endif

    if (istat & ISR_OVFLO)
	return ne2_overflow(buf, length);

    if (next_page != curr_page)
	return ne2_read_pkt(buf, length);

    return 0;
}

int
ne2_irq_disable(void *unused)
{
    int was_enabled;

    was_enabled = NE2_READ(PG0_IMR) & IMR_VAL;
    NE2_WRITE(0, PG0_IMR);

    return was_enabled;
}


static void
ne2_irq_enable(void *unused)
{
    NE2_WRITE(IMR_VAL, PG0_IMR);
}


int
ne2_irq_handler(int irq_nr, void *regs)
{
    extern int gdb_interrupt_check(void);
    int curr_page, i, rval;

    i = NE2_READ(PG0_ISR);

    NE2_WRITE(i & (ISR_RX|ISR_RXERR), PG0_ISR);

    rval = 1;
    for (i = 0; i < 10; i++) {
	NE2_WRITE(CMD_NODMA|CMD_PAGE1|CMD_START, NE2_CMD);
	curr_page = NE2_READ(PG1_CURR);
	NE2_WRITE(CMD_NODMA|CMD_PAGE0|CMD_START, NE2_CMD);

	if (next_page != curr_page) {
	    if (gdb_interrupt_check()) {
		BSPLOG(bsp_log("Got ^C.\n"));
		rval = 0;
		break;
	    }
	}
    }

    mb86941_write(DB_BASE, MB86941_IRQCLR, (2 << (NE2_IRQ - BSP_EXC_INT1)));
    mb86941_write(DB_BASE, MB86941_IRLATCH, 0x10);

    return rval;
}



static int
ne2_control(void *unused, int func, ...)
{
    static bsp_vec_t eth_vec;
    int          retval = 0;
    va_list      ap;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_INSTALL_DBG_ISR:
	eth_vec.handler = ne2_irq_handler;
	eth_vec.next = NULL;

	bsp_install_vec(BSP_VEC_EXCEPTION, NE2_IRQ,
			BSP_VEC_REPLACE, &eth_vec);
	bsp_enable_irq(NE2_IRQ);
	ne2_irq_enable(unused);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	ne2_irq_disable(unused);
	bsp_disable_irq(NE2_IRQ);
	bsp_remove_vec(BSP_VEC_EXCEPTION, NE2_IRQ, &eth_vec);
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = ne2_irq_disable(unused);
	break;

      case COMMCTL_IRQ_ENABLE:
	ne2_irq_enable(unused);
	break;

      default:
	retval = -1;
	break;
    }

    va_end(ap);
    return retval;
}


static void
dummy_putc(void *unused, char ch)
{
#if 0
    bsp_printf("putc not supported for an enet link driver\n");
#endif
}

static int
dummy_getc(void *unused)
{
#if 0
    bsp_printf("getc not supported for an enet link driver\n");
#endif
    return 0;
}


static struct bsp_comm_procs ne2_procs = {
    NULL,
    ne2_send, ne2_receive,
    dummy_putc, dummy_getc,
    ne2_control
};


struct bsp_comm_procs *
__enet_link_init(void)
{
    if (ne2_init())
	return &ne2_procs;
    return NULL;
}

/* PCI interrupt enables */
#define PCI_PLX_IEN   0x00100000
#define PCI_SLOT4_IEN 0x00080000
#define PCI_SLOT3_IEN 0x00040000
#define PCI_SLOT2_IEN 0x00020000
#define PCI_SLOT1_IEN 0x00010000

#define PCI_IEN_REG *((volatile unsigned *)0x56500080)

static struct {
    int cfga;
    int ien;
} slot_tab[] = {
    { 0x80000000, PCI_SLOT1_IEN },
    { 0x80000800, PCI_SLOT2_IEN },
    { 0x80001000, PCI_SLOT3_IEN },
    { 0x80001800, PCI_SLOT4_IEN }
};


int
__enet_pci_probe(void)
{
    unsigned short vendor, device;
    int i;

    /* search all four slots for first Realtek card. */
    for (i = 0; i < 4; i++) {

	PLX_DMCFGA = slot_tab[i].cfga;

	vendor = *(volatile unsigned short *)(PLX_PCIMEM_BASE+2);
	device = *(volatile unsigned short *)(PLX_PCIMEM_BASE);

	if (vendor == 0x10ec && device == 0x8029) {
	    bsp_printf("Found RTL8029 in slot%d\n", i+1);

	    PLX_DMCFGA = slot_tab[i].cfga | 0x10;

	    *(volatile unsigned *)(PLX_PCIMEM_BASE) = 0xffffffff;
	    *(volatile unsigned *)(PLX_PCIMEM_BASE) = 0;

	    /* enable IO in PCI command register */
	    PLX_DMCFGA = slot_tab[i].cfga | 0x04;
	    *(volatile unsigned short *)(PLX_PCIMEM_BASE+2) = 1;

	    PLX_DMCFGA = 0x00000000;

	    /* set bigendian mode for direct master accesses */
	    PLX_BIGEND = 2;

	    /* enable interrupts from the pci slot */
	    PCI_IEN_REG = slot_tab[i].ien;

	    return 1;
	}
    }

    return 0;
}

