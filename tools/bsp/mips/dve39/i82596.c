/*
 * i82596.c -- Driver for Intel 82596 Ethernet Controller
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
#include <string.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/dve39.h>
#include "bsp_if.h"
#include "i82596.h"
#include "net.h"
#include "bsplog.h"

#if 0
#define DEBUG_ENET 32
#endif

#if DEBUG_ENET
static void DumpStatus(void);
#endif

/* number of ~1540 byte rx buffers and descriptors */
#define NUM_RX_BUFFS 8

/*
 *  Receive Frame Descriptors
 */
static struct rfd   __rbufs[NUM_RX_BUFFS];
static struct rfd   *rx_next;

/*
 *  Single transmit buffer.
 */
static struct tx __tx;
static struct tx *txp;

/*
 * The scp is declared as an array of bytes so that we can adjust it to a
 * 16 byte boundary as required by the i82596.
 */
static char        _scp[sizeof(struct scp) + 15];
static struct iscp _iscp;
static struct scb  _scb;
 
static struct scp   *scp;
static struct iscp  *iscp;
static struct scb   *scb;

enet_addr_t __local_enet_addr;           /* local ethernet address */

static inline void
i596_wait(int n)
{
    int i = n;

    while (scb->command) {
	if (--i <= 0) {
	    bsp_printf("timed out waiting on prior scb->command <0x%x>\n",
			scb->command);
	    i = n;
	}
    }
}


/*
 * Have the i82596 execute the given command. The cmd data must be uncached
 * as is the scb.
 */
static inline void
i596_do_cmd(struct cmd *cmd)
{
    unsigned short command;

    cmd->command |= CMD_EL;
    cmd->status = 0;
    cmd->link = 0;

    command = SCB_CUC_START | (scb->status & (SCB_CX|SCB_CNA));

    /*
     * wait for prior commands to finish.
     */
    i596_wait(10000);

    scb->cba = I596_ADDR(cmd);
    scb->command = command;

    I596_CHANNEL_ATTN();
}

/* 
 * Initialize the Ethernet Interface.
 */
static void
i596_init(enet_addr_t enet_addr)
{
    struct rfd         *rfd;
    union {
	struct config_cmd config;
	struct ia_cmd     ia;
	struct tdr_cmd    tdr;
    } u;
    struct config_cmd  *cfgp;
    struct ia_cmd      *iap;
    int                i;

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Initializing i82896...\n"));
#endif

    memcpy(__local_enet_addr, enet_addr, sizeof(enet_addr_t));

    I596_RESET();

    /*
     * initialize pointers to i596 control structures using uncached
     * addresses. Note the scp must be aligned to a 16 byte boundary.
     */
    scp  = (struct scp *)CPU_UNCACHED(&_scp[0]);
    scp  = (struct scp *)(((unsigned long)scp + 15) & ~15);
    iscp = (struct iscp *)CPU_UNCACHED(&_iscp);

    scb  = (struct scb *)CPU_UNCACHED(&_scb);
    memset(scb, 0, sizeof(*scb));
    
    txp  = (struct tx *)CPU_UNCACHED(&__tx);
    memset(txp, 0, sizeof(*txp));
    txp->tbd.txbuf = I596_ADDR((void*)txp->data);
    txp->tbd.next = 0;
    txp->cmd.link = 0;
    txp->size = 0;
    txp->tbp = I596_ADDR(&txp->tbd);

    /*
     *  initialize receive buffers
     */
    rfd = CPU_UNCACHED(&__rbufs[0]);

    memset(rfd, 0, sizeof(*rfd));

    for (i = 0; i < (NUM_RX_BUFFS-1); i++, rfd++) {
	rfd->size = sizeof(rfd->data);
	rfd->link = I596_ADDR((rfd+1));
    }
    rfd->size = sizeof(rfd->data);
    rfd->link = scb->rfa = I596_ADDR(&__rbufs[0]);
    
    rx_next = I596_CPU_UNCACHED(scb->rfa);

    /* fill out scp */
    memset(scp, 0, sizeof(*scp));
    scp->sysbus = SYSBUS_LINEAR | 0x40;
    scp->iscp   = I596_ADDR(iscp);

    /* fill out iscp */
    memset(iscp, 0, sizeof(*iscp));
    iscp->busy = 1;
    iscp->scb  = I596_ADDR(scb);

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Setting SCP...\n"));
#endif
    I596_SET_SCP(scp);

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Sending Channel Attn...\n"));
#endif
    I596_CHANNEL_ATTN();    

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Waiting for busy flag...\n"));
#endif
    /* wait for busy flag to clear */
    i = 1000000;
    while (iscp->busy) {
	if (--i <= 0) {
	    bsp_printf("i82596 failed to initialize\n");
	    while (1) ;
	}
    }

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Done.\n"));
#endif

    scb->command = 0;

    cfgp = (struct config_cmd *)CPU_UNCACHED(&u.config);
    cfgp->cmd.status = 0;
    cfgp->cmd.command = CMD_CONFIGURE;
    cfgp->cmd.link = NULL;
    cfgp->data[0]  = 0x8e;	/* length, prefetch on */
    cfgp->data[1]  = 0xc8;	/* fifo to 8, monitor off */
    cfgp->data[2]  = 0x40;	/* don't save bad frames */
    cfgp->data[3]  = 0x2e;	/* No src address insertion, 8 byte preamble */
    cfgp->data[4]  = 0x00;	/* priority and backoff defaults */
    cfgp->data[5]  = 0x60;	/* interframe spacing */
    cfgp->data[6]  = 0x00;	/* slot time LSB */
    cfgp->data[7]  = 0xf2;	/* slot time and retries */
    cfgp->data[8]  = 0x00;	/* promiscuous mode */
    cfgp->data[9]  = 0x00;	/* collision detect */
    cfgp->data[10] = 0x40;	/* minimum frame length */
    cfgp->data[11] = 0xff;
    cfgp->data[12] = 0x00;
    cfgp->data[13] = 0x3f;	/*  single IA */
    cfgp->data[14] = 0x00;
    cfgp->data[15] = 0x00;
#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Sending CONFIG command to i82596...\n"));
#endif
    i596_do_cmd(&cfgp->cmd);
    i596_wait(10000);

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Done.\n"));
#endif
    iap = (struct ia_cmd *)CPU_UNCACHED(&u.ia);
    iap->cmd.status = 0;
    iap->cmd.command = CMD_IASETUP;
    iap->cmd.link = NULL;
    memcpy(iap->eth_addr, __local_enet_addr, 6);

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Sending IASET command to i82596...\n"));
#endif
    i596_do_cmd(&iap->cmd);
    i596_wait(10000);

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Starting i82596 Receive...\n"));
#endif

#if 0
    {
	struct tdr_cmd     *tdrp;

	tdrp = (struct tdr_cmd *)CPU_UNCACHED(&u.tdr);
	tdrp->cmd.status = 0;
	tdrp->cmd.command = CMD_TDR;
	tdrp->cmd.link = NULL;
	tdrp->result = 0;
	i596_do_cmd(&tdrp->cmd);
	i596_wait(10000);

	while (!(tdrp->cmd.status & 0x8000))
	    ;
	bsp_printf("tdr[%08x]\n", tdrp->result);
    }
#endif

    scb->command = SCB_RUC_START;
    I596_CHANNEL_ATTN();
    i596_wait(10000);

#if DEBUG_ENET >= 3
    BSPLOG(bsp_log("Done.\n"));

    DumpStatus();
#endif
}


/*
 *  Send a packet out over the ethernet.  The packet is sitting at the
 *  beginning of the transmit buffer.  The routine returns when the
 *  packet has been successfully sent.
 */
static void
i596_send(void *unused, const char *buf, int length)
{
    if (length < ETH_MIN_PKTLEN) 
        length = ETH_MIN_PKTLEN; /* and min. ethernet len */
    else if (length > I596_BUFSIZE)
	return;

    memcpy(((eth_header_t *)buf)->source, __local_enet_addr,
	   sizeof(__local_enet_addr));

#if (DEBUG_ENET >= 16)
    BSPLOG(bsp_log("i596_send: len[%d]\n", length));
#endif

    i596_wait(10000);

    txp->cmd.status = 0;
    txp->cmd.command = CMD_XMIT | CMD_FLEXMODE;
    txp->tbd.size = length | TBD_EOF;

    /*
     * Make sure cache and main memory are coherent.
     */
    memcpy(txp->data, buf, length);

    i596_do_cmd(&txp->cmd);
}


/* 
 * Test for the arrival of a packet on the Ethernet interface.
 * If a packet is found, copy it into the buffer and return its length.
 * If no packet, then return 0.
 */
int
i596_receive(void *unused, char *buf, int maxlen)
{
    int   pktlen;
    short scb_status, command;
    struct rfd *rfd;

    rfd = rx_next;

    command = scb->command;
    scb_status = scb->status;

#if 0
    BSPLOG(bsp_log("i596_rx: cmd[%04x] sts[%04x]\n",
		   command, scb_status));

    {
	struct rfd *p;
	int i;

	p = rfd;
	for (i = 0; i < NUM_RX_BUFFS; i++) {
	    BSPLOG(bsp_log("       : rfd[%x] cmd[%04x] sts[%04x]\n",
			   p, p->command, p->status));
	    p = I596_CPU_UNCACHED(p->link);
	}
    }
#endif

    /* return if nothing received */
    if ((rfd->status & CMD_C) == 0) {
	/* but first, restart rx if necessary */
	if (scb_status & SCB_RNR) {
	    i596_wait(10000);
	    scb->command = SCB_CUC_NOP | SCB_RUC_START | SCB_ACK_RNR;
	    I596_CHANNEL_ATTN();
	}
	return 0;
    }

    /* a packet was received, check if it was good */
    if (rfd->status & CMD_OK) {
	pktlen = rfd->count & 0x3fff;
	if (pktlen > maxlen)
	    pktlen = maxlen;

	memcpy(buf, rfd->data, pktlen);

	rfd->status = 0;
	rfd->command = 0;
	rfd->count = 0;
	rfd->rbd = 0;

#if DEBUG_ENET > 1
	BSPLOG(bsp_log("got pkt - type 0x%x len[%d]\n",
		       ((eth_header_t *)buf)->type,
	               pktlen));
#endif
    } else
	pktlen = 0;

    rx_next = I596_CPU_UNCACHED(rfd->link);

    return pktlen;
}


static int
i596_irq_handler(int irq_nr, void *regs)
{
    extern int gdb_interrupt_check(void);
    unsigned short status, command;

    I596_IRQ_CLEAR();

    status = scb->status;

    BSPLOG(bsp_log("ether irq: status[%04x]\n", status));

    if (status & (SCB_FR | SCB_RNR)) {

	i596_wait(10000);
	
	command = SCB_CUC_NOP | SCB_RUC_NOP;
	if (status & SCB_FR)
	    command |= SCB_ACK_FR;
	if (status & SCB_RNR)
	    command |= (SCB_ACK_RNR | SCB_RUC_START);

	scb->command = command;
	I596_CHANNEL_ATTN();

	if (status & SCB_FR) {
	    if (gdb_interrupt_check()) {
		BSPLOG(bsp_log("Got ^C.\n"));
		return 0;
	    }
	}
    }

    return 1;
}


static int
i596_control(void *unused, int func, ...)
{
    static bsp_vec_t eth_vec;
    int          retval = 0;
    va_list      ap;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_INSTALL_DBG_ISR:
	eth_vec.handler = i596_irq_handler;
	eth_vec.next = NULL;

	bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_ETHER,
			BSP_VEC_REPLACE, &eth_vec);
	bsp_enable_irq(BSP_IRQ_ETHER);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	bsp_disable_irq(BSP_IRQ_ETHER);
	bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_ETHER, &eth_vec);
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = bsp_disable_irq(BSP_IRQ_ETHER);
	break;

      case COMMCTL_IRQ_ENABLE:
	bsp_enable_irq(BSP_IRQ_ETHER);
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
    bsp_printf("putc not supported for an enet link driver\n");
}

static int
dummy_getc(void *unused)
{
    bsp_printf("getc not supported for an enet link driver\n");
}


static struct bsp_comm_procs i596_procs = {
    NULL,
    i596_send, i596_receive,
    dummy_putc, dummy_getc,
    i596_control
};


struct bsp_comm_procs *
__enet_link_init(enet_addr_t enet_addr)
{
    i596_init(enet_addr);
    return &i596_procs;
}


#if DEBUG_ENET
static void
DumpStatus(void)
{
    BSPLOG(bsp_log("scb: command[0x%x] status[0x%x]\n",
		scb->command, scb->status));
}
#endif
