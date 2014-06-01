/*
 * mbx.c -- MBX initialization.
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
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "queue.h"

/*
 *  QSPAN PCI interface ASIC.
 */
struct qspan {
    volatile unsigned int	pci_id;
    volatile unsigned int	pci_cs;
    volatile unsigned int	pci_class;
    volatile unsigned int	pci_misc0;
    volatile unsigned int	pci_bsm;
    volatile unsigned int	pci_bsio;
    volatile unsigned int	RESERVED1[5];
    volatile unsigned int	pci_sid;
    volatile unsigned int	pci_bsrom;
    volatile unsigned int	RESERVED2[2];
    volatile unsigned int	pci_misc1;
    volatile unsigned int	RESERVED3[48];
    volatile unsigned int	pbti0_ctl;
    volatile unsigned int	pbti0_add;
    volatile unsigned int	RESERVED4[2];
    volatile unsigned int	pbti1_ctl;
    volatile unsigned int	pbti1_add;
    volatile unsigned int	RESERVED5[9];
    volatile unsigned int	pbrom_ctl;
    volatile unsigned int	pb_errcs;
    volatile unsigned int	pb_aerr;
    volatile unsigned int	pb_derr;
    volatile unsigned int	RESERVED6[173];
    volatile unsigned int	idma_cs;
    volatile unsigned int	idma_add;
    volatile unsigned int	idma_cnt;
    volatile unsigned int	RESERVED7[61];
    volatile unsigned int	con_add;
    volatile unsigned int	con_data;
    volatile unsigned int	RESERVED8[62];
    volatile unsigned int	int_stat;
    volatile unsigned int	int_ctl;
    volatile unsigned int	int_dir;
    volatile unsigned int	RESERVED9[125];
    volatile unsigned int	misc_ctl;
    volatile unsigned int	RESERVED10[447];
    volatile unsigned int	qbsi0_ctl;
    volatile unsigned int	qbsi0_at;
    volatile unsigned int	RESERVED11[2];
    volatile unsigned int	qbsi1_ctl;
    volatile unsigned int	qbsi1_at;
    volatile unsigned int	RESERVED12[3];
    volatile unsigned int	qb_errcs;
    volatile unsigned int	qb_aerr;
    volatile unsigned int	qb_derr;
};


#if 0
static void init_qspan(void)
{
    struct qspan *q = (struct qspan *)0xfa210000;

    /*
     *  Initialization values taken from:
     *  Table 56: QSPAN Initialization Values
     *  "Engineering Specification for MBX Series."
     */
    q->pci_cs    = 0x02800147;
    q->pci_misc0 = 0x00000000;
    q->pci_bsm   = 0x01000000;
    q->pci_bsio  = 0x1ffff001;
    q->pci_bsrom = 0x00000000;
    q->pci_misc1 = 0x00000100;
    q->pbti0_ctl = 0x8f000080;
    q->pbti0_add = 0x80000000;
    q->pbti1_ctl = 0x00000000;
    q->pbti1_add = 0x00000000;
    q->pb_errcs  = 0x80000000;
    q->idma_cs   = 0x00780008;
    q->int_stat  = 0xef000003;
    q->int_ctl   = 0x00000000;
    q->int_dir   = 0x00000000;
    q->misc_ctl  = 0x00000003;
    q->qbsi0_ctl = 0x81000000;
    q->qbsi0_at  = 0x000000f1;
    q->qbsi1_ctl = 0x80000000;
    q->qbsi1_at  = 0x000000f1;
    q->qb_errcs  = 0x81000000;
}
#endif


#define Rxbd     0x2800       /* Rx Buffer Descriptor Offset */
#define Txbd     0x2808       /* Tx Buffer Descriptor Offset */

#define Rxbuf    ((volatile char *)eppc + 0x2810)
#define Txbuf    ((volatile char *)eppc + 0x2820)

/*
 *  Initialize SMC1 as a uart.
 *
 *  Comments below reference Motorola's "MPC860 User Manual".
 *  The basic initialization steps are from Section 16.15.8
 *  of that manual.
 */	
static void
init_smc1_uart(void)
{
    int dcache_was_enabled;
    EPPC *eppc;
    volatile struct smc_uart_pram *uart_pram;
    struct cp_bufdesc *txbd, *rxbd;

    dcache_was_enabled = __dcache_disable();

    eppc = eppc_base();

    /* SMC1 Uart paramater ram */
    uart_pram = &eppc->pram[2].scc.pothers.smc_modem.psmc.u;

    /* tx and rx buffer descriptors */
    txbd = (struct cp_bufdesc *)((char *)eppc + Txbd);
    rxbd = (struct cp_bufdesc *)((char *)eppc + Rxbd);

    /*
     *  Set up the PortB pins for UART operation.
     *  Set PAR and DIR to allow SMCTXD1 and SMRXD1
     *  (Table 16-39)
     */
    eppc->pip_pbpar |= 0xc0;
    eppc->pip_pbdir &= ~0xc0;

    /*
     *  Configure BRG for 9600 baud @ 40MHz sysclk
     *  (Section 16.13.2)
     */
    eppc->brgc1 = 0x10204 /*0x10082*/;

    /*
     *  NMSI mode, BRG1 to SMC1
     *  (Section 16.12.5.2)
     */
    eppc->si_simode = 0;

    /*
     *  Set pointers to buffer descriptors.
     *  (Sections 16.15.4.1, 16.15.7.12, and 16.15.7.13)
     */
    uart_pram->rbase = Rxbd;
    uart_pram->tbase = Txbd;

    /*
     *  Init Rx & Tx params for SMC1
     */
    eppc->cp_cr = 0x91;

    /*
     *  SDMA & LCD bus request level 5
     *  (Section 16.10.2.1)
     */
    eppc->dma_sdcr = 1;

    /*
     *  Set Rx and Tx function code
     *  (Section 16.15.4.2)
     */
    uart_pram->rfcr = 0x18;
    uart_pram->tfcr = 0x18;

    /* max receive buffer length */
    uart_pram->mrblr = 1;

    /* disable max_idle feature */
    uart_pram->max_idl = 0;

    /* no last brk char received */
    uart_pram->brkln = 0;

    /* no break condition occurred */
    uart_pram->brkec = 0;

    /* 1 break char sent on top XMIT */
    uart_pram->brkcr = 1;

    /* setup RX buffer descriptor */
    rxbd->length = 0;
    rxbd->buffer = Rxbuf;
    rxbd->ctrl   = 0xb000;

    /* setup TX buffer descriptor */
    txbd->length = 1;
    txbd->buffer = Txbuf;
    txbd->ctrl   = 0x2000;

    /*
     *  Clear any previous events. Mask interrupts.
     *  (Section 16.15.7.14 and 16.15.7.15)
     */
    eppc->smc_regs[0].smc_smce = 0xff;
    eppc->smc_regs[0].smc_smcm = 5;

    /*
     *  Set 8,n,1 characters, then also enable rx and tx.
     *  (Section 16.15.7.11)
     */
    eppc->smc_regs[0].smc_smcmr = 0x4820;
    eppc->smc_regs[0].smc_smcmr = 0x4823;

    if (dcache_was_enabled)
	__dcache_enable();
}


#define UART_BUFSIZE 32

static bsp_queue_t uart_queue;
static char uart_buffer[UART_BUFSIZE];

static void
uart_putchar(void *which, char ch)
{
    struct cp_bufdesc *bd;

    /* tx buffer descriptors */
    bd = (struct cp_bufdesc *)((char *)eppc_base() + Txbd);

    while (bd->ctrl & 0x8000) ;

    bd->buffer[0] = ch;
    bd->length    = 1;
    bd->ctrl      = 0xa000;
}


static void
uart_write(void *which, const char *buf, int len)
{
    while (len-- > 0)
	uart_putchar(which, *buf++);
}

static int
uart_rcvchar(void *which)
{
    struct cp_bufdesc *bd;
    char ch;

    /* rx buffer descriptors */
    bd = (struct cp_bufdesc *)((char *)eppc_base() + Rxbd);

    while (bd->ctrl & 0x8000) ;

    ch = bd->buffer[0];

    bd->length = 1;
    bd->ctrl   = 0xb000;

    return ch;
}


static int
uart_getchar(void *which)
{
    int was_enabled;
    int ch;

    was_enabled = bsp_disable_irq(BSP_IRQ_SMC1);
    ch = _bsp_dequeue(&uart_queue);
    if (ch < 0)
	ch = uart_rcvchar(which);
    if (was_enabled)
	bsp_enable_irq(BSP_IRQ_SMC1);
    return ch;
}

static int
uart_read(void *which, char *buf, int len)
{
    int ch;

    if (len <= 0)
	return 0;

    ch = uart_getchar(which);
    *buf = ch;
    return 1;
}


static int uart_baud;

struct _baud {
    unsigned int baud;
    unsigned int val;
};

const static struct _baud bauds[] = {
    { 9600,  0x10204 },
    { 19200, 0x10102 },
    { 38400, 0x10082 }
};

static int
uart_setbaud(void *base, int bps)
{
    int   i;
    const struct _baud *bp;

    for (i = 0, bp = bauds; i < (sizeof(bauds)/sizeof(bauds[0])); i++, bp++)
	if (bp->baud == bps) {
	    eppc_base()->brgc1 = bp->val;
	    uart_baud = bps;
	    return 0;
	}
    return -1;
}

static int
smc1_irq_handler(int irq_nr, void *regs)
{
    EPPC *eppc = eppc_base();
    struct cp_bufdesc *bd;
    char ch;

    eppc->smc_regs[0].smc_smce = 0xff;

    /* rx buffer descriptors */
    bd = (struct cp_bufdesc *)((char *)eppc_base() + Rxbd);

    if ((bd->ctrl & 0x8000) == 0) {
	ch = uart_rcvchar(NULL);
	if (ch == '\003') {
	    eppc->cpmi_cisr = 0x10;
	    return 0;
	}
	_bsp_enqueue(&uart_queue, ch);
    }
    eppc->cpmi_cisr = 0x10;
    return 1;
}



static int
uart_control(void *which, int func, ...)
{
    int          i, retval = 0;
    va_list      ap;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(which, i);
	break;

      case COMMCTL_GETBAUD:
	retval = uart_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = smc1_irq_handler;
	uart_vec.next    = NULL;
	bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SMC1,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(BSP_IRQ_SMC1);
	return 0;
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	bsp_disable_irq(BSP_IRQ_SMC1);
	bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SMC1, &uart_vec);
	return 0;
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = bsp_disable_irq(BSP_IRQ_SMC1);
	break;

      case COMMCTL_IRQ_ENABLE:
	bsp_enable_irq(BSP_IRQ_SMC1);
	break;

      default:
	retval = -1;
	break;
    }

    va_end(ap);
    return retval;
}



struct bsp_comm_channel _bsp_comm_list[] = 
{
    {
	{ "smc1", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ NULL, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
};


int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);


#if 0
static void
init_ports(void)
{
    EPPC *eppc = eppc_base();

    eppc->pio_papar = 0x0a0f;
    eppc->pio_padir = 0x0000;
    eppc->pio_paodr = 0x000a;

    eppc->pip_pbpar = 0x00f0;
    eppc->pip_pbdir = 0x0030;
    eppc->pip_pbodr = 0x0030;

    eppc->pio_pcpar = 0x0003;
    eppc->pio_pcdir = 0x0000;
    eppc->pio_pcso  = 0x0000;

    eppc->pio_pdpar = 0x03ff;
    eppc->pio_pddir = 0x00af;
}
#endif


/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    EPPC *eppc = eppc_base();
    int i;

    /*
     *  Reset communications processor
     */
    eppc->cp_cr = 0x8001;
    for (i = 0; i < 100000; i++);

#if 0
    init_ports();
#endif

    _bsp_queue_init(&uart_queue, uart_buffer, sizeof(uart_buffer));
    init_smc1_uart();

#if 0
    init_qspan();
#endif
}



void
_bsp_install_board_debug_traps(void)
{
}

void
_bsp_install_board_irq_controllers(void)
{
}


/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)0, (void *)0x0, 0, 0, BSP_MEM_RAM }
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = 1;

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    /*
     * Finish setup of RAM description. Early initialization put a
     * physical pointer to the top of RAM in _bsp_ram_info_ptr.
     */
    _bsp_memory_list[0].nbytes = (long)_bsp_ram_info_ptr;

    /*
     * Override default platform info.
     */
    _bsp_platform_info.cpu = "PPC860";
    _bsp_platform_info.board = "Motorola MBX";

    /*
     * Override cache info.
     */
#if 0
    _bsp_dcache_info.size = 1024;
    _bsp_dcache_info.linesize = 4;
    _bsp_dcache_info.ways = 2;

    _bsp_icache_info.size = 4096;
    _bsp_icache_info.linesize = 16;
    _bsp_icache_info.ways = 1;
#endif

    __decr_cnt = 3000;  /* !!!FIXME!!! */
}

