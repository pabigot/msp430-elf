/*
 * eval1.c -- Support for Fujitsu MB91V101 eval board.
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

#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "queue.h"
#include "net.h"
#include <stdlib.h>

#define DEBUG_CHAN  fr30_io->uart0
#define DEBUG_UTIM  fr30_io->utimer0

#define RTIMER      (&fr30_io->rtimer2)

#define UART_BUFSIZE 32

static bsp_queue_t uart_queue;
static char uart_buffer[UART_BUFSIZE];

/*
 * Initialize the ethernet interface.
 * Returns a pointer to bsp_comm_procs vectors for ethernet i/o.
 */
extern struct bsp_comm_procs *__enet_link_init(enet_addr_t enet_addr);


static void
uart_irq_enable(fr30_uart_t *chan)
{
    chan->ssr |= UART_SSR_RIE;
}

static int
uart_irq_disable(fr30_uart_t *chan)
{
    if (chan->ssr & UART_SSR_RIE) {
	chan->ssr &= ~UART_SSR_RIE;
	return 1;
    }
    return 0;
}


static void
uart_putchar(void *chan_data, char ch)
{
    fr30_uart_t *chan = chan_data;

    while (!(chan->ssr & UART_SSR_TDRE))
	;

    chan->data = ch;
}


static void
uart_write(void *chan_data, const char *buf, int len)
{
    while (len-- > 0)
	uart_putchar(chan_data, *buf++);
}


static int
uart_read(void *chan_data, char *buf, int len)
{
    fr30_uart_t *chan = chan_data;

    if (chan->ssr & UART_SSR_RDRF) {
	*buf = chan->data;
	return 1;
    }

    return 0;
}

static int
uart_rcvchar(void *chan_data)
{
    unsigned char ch;

    while (uart_read(chan_data, (char *)&ch, 1) != 1)
	;
    return ch;
}


static int
uart_getchar(void *chan_data)
{
    int ch, enabled;

    enabled = uart_irq_disable(chan_data);
    ch = _bsp_dequeue(&uart_queue);
    if (ch < 0)
	ch = uart_rcvchar(chan_data);
    if (enabled)
	uart_irq_enable(chan_data);
    return ch;
}


static int
uart_irq_handler(int irq_nr, void *regs)
{
    unsigned char ch;

    if (uart_read(&DEBUG_CHAN, (char *)&ch, 1) == 1) {
	if (ch == '\003') {
	    /* make sure nothing is in the queue */
	    while (_bsp_dequeue(&uart_queue) >= 0)
		;
	    return 0;
	} else
	    _bsp_enqueue(&uart_queue, ch);
    }
    return 1;
}

struct _baud {
    unsigned short  baud;
    unsigned short  val;
};

#define USE_UCC1  0x8000

static const struct _baud _bauds_25[] = {
    {   2400, 324 | USE_UCC1 },
    {   4800, 162 },
    {   9600,  80 | USE_UCC1 },
    {  19200,  39 | USE_UCC1 },
    {  38400,  19 | USE_UCC1 },
    {  57600,  12 | USE_UCC1 },
    { 0, 0 }
};

static const struct _baud _bauds_12[] = {
    {   2400, 162 | USE_UCC1 },
    {   4800,  80            },
    {   9600,  39 | USE_UCC1 },
    {  19200,  19 | USE_UCC1 },
    {  38400,   9 | USE_UCC1 },
    { 0, 0 }
};

static const struct _baud _bauds_6[] = {
    {   2400,  81 | USE_UCC1 },
    {   4800,  40            },
    {   9600,  19 | USE_UCC1 },
    {  19200,   9 | USE_UCC1 },
    { 0, 0 }
};

static const struct _baud *_bauds;

static int uart_baud;

static int
uart_setbaud(fr30_uart_t *chan, int bps)
{
    int i;
    fr30_utimer_t *utim = &fr30_io->utimer0;

    for (i = 0; _bauds[i].baud; i++) {
	if (_bauds[i].baud == bps) {
	    /* stop timer and clear UCC1 bit */
	    utim->utimc &= ~(UTIMER_UTST | UTIMER_UCC1);
	    if (_bauds[i].val & USE_UCC1) {
		utim->utim  = _bauds[i].val & 0x7fff;
		utim->utimc |= UTIMER_UCC1;
	    } else
		utim->utim  = _bauds[i].val;
	    /* start timer */
	    utim->utimc |= UTIMER_UTST;

	    uart_baud = bps;
	    return 0;
	}
    }
    return -1;
}


static int
uart_control(void *chan_data, int func, ...)
{
    int          i, retval = 0;
    va_list      ap;
    fr30_uart_t *chan = chan_data;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(chan, i);
	break;

      case COMMCTL_GETBAUD:
	retval = uart_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = uart_irq_handler;
	uart_vec.next = NULL;

	bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_UART0_RX,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(BSP_EXC_UART0_RX);

	uart_irq_enable(chan);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	uart_irq_disable(chan);
	bsp_disable_irq(BSP_EXC_UART0_RX);
	bsp_remove_vec(BSP_VEC_EXCEPTION, BSP_EXC_UART0_RX, &uart_vec);
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = uart_irq_disable(chan);
	break;

      case COMMCTL_IRQ_ENABLE:
	uart_irq_enable(chan);
	break;

      default:
	retval = -1;
	break;
    }

    va_end(ap);
    return retval;
}


static void
init_uart(int which, int bps)
{
    fr30_uart_t *chan;

    _bsp_queue_init(&uart_queue, uart_buffer, sizeof(uart_buffer));

    chan = &DEBUG_CHAN;

    /* disable and clear flags */
    chan->scr = 0;

    /* clear interrupt enable flags */
    chan->ssr = 0;

    /* async mode 0, enable data out pin */
    chan->smr = UART_SMR_ASYNC | UART_SMR_SOE;
    chan->scr = UART_SCR_8BIT | UART_SCR_1STOP | UART_SCR_NONE |
		UART_SCR_TXE | UART_SCR_RXE;

    uart_setbaud(chan, bps);
}


struct bsp_comm_channel _bsp_comm_list[] = 
{
    {
	{ "uart1", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)&DEBUG_CHAN, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
};


int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);

#ifdef BSP_LOG
int bsp_dump_flag;
#endif

/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    volatile unsigned char pll, pclk;
    int i;

    pll = (fr30_io->pctr >> 6) & 3;
    pclk = (fr30_io->gcr >> 2) & 3;

    switch (pll) {
      case 0:
	switch (pclk) {
	  case 0:
	    _bauds = _bauds_12;
	    break;
	  case 1:
	    _bauds = _bauds_6;
	    break;
	}
	break;
      case 1:
	switch (pclk) {
	  case 0:
	    _bauds = _bauds_25;
	    break;
	  case 1:
	    _bauds = _bauds_12;
	    break;
	  case 2:
	    _bauds = _bauds_6;
	    break;
	}
	break;
      case 2:
      case 3:
	_bauds = _bauds_25;
	break;
    }
    
    for (i = 0; _bauds[i+1].baud; i++)
	;

#ifndef BSP_LOG
    init_uart(0, _bauds[i].baud);
#else
    init_uart(0,19200);
    bsp_dump_flag = (~ (*(volatile unsigned char *)0x00304000)) & 0x80;
#endif
}


/*
 * Install any board specific interrupt controllers.
 */
void
_bsp_install_board_irq_controllers(void)
{
}


/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)0x01000000, (void *)0x01000000, 0, 4 * 1024*1024, BSP_MEM_RAM },
    { (void *)0x000a0000, (void *)0x000a0000, 0, 256 * 1024, BSP_MEM_FLASH }
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = 2;



static unsigned long ticks;
static unsigned short last_count;
static int            count_per_ms;


#define COUNT_25  781
#define COUNT_12  390
#define COUNT_6   195


unsigned long
_bsp_ms_ticks(void)
{
    unsigned short current;
    static unsigned short diff = 0;

    current = RTIMER->tmr;

    /* its a down counter, so use "last - current" */
    diff += last_count - current;

    if (diff >= count_per_ms) {
	ticks += (diff / count_per_ms);
	diff = (diff % count_per_ms);
    }

    last_count = current;

    return ticks;
}


unsigned short
_bsp_fast_ticks(void)
{
    return RTIMER->tmr;
}


/*
 * Setup reload timer 2 to be a free running counter
 * that the network stack uses for timing.
 */
void
init_rtimer(void)
{
    fr30_rtimer_t *rt = RTIMER;
    unsigned short dummy;
    volatile unsigned char pll, pclk;

    pll = (fr30_io->pctr >> 6) & 3;
    pclk = (fr30_io->gcr >> 2) & 3;

    switch (pll) {
      case 0:
	switch (pclk) {
	  case 0:
	    count_per_ms = COUNT_12;
	    break;
	  case 1:
	    count_per_ms = COUNT_6;
	    break;
	}
	break;
      case 1:
	switch (pclk) {
	  case 0:
	    count_per_ms = COUNT_25;
	    break;
	  case 1:
	    count_per_ms = COUNT_12;
	    break;
	  case 2:
	    count_per_ms = COUNT_6;
	    break;
	}
	break;
      case 2:
      case 3:
	count_per_ms = COUNT_25;
	break;
    }

    rt->tmrlr = 0xfffe;	/* reload value */

    rt->tmcsr = RTIMER_CLK32 | RTIMER_RELD | RTIMER_CNTE | RTIMER_TRG;

    dummy = rt->tmr;

    (void)_bsp_ms_ticks();
}

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    enet_addr_t   addr;
    unsigned char sw1;
    struct bsp_comm_procs *eprocs;


    /*
     * Override default platform info.
     */
    _bsp_platform_info.cpu = "MB91V101";
    _bsp_platform_info.board = "Fujitsu MB91901EB";

    /*
     * Override cache info.
     */
    _bsp_icache_info.size = 1024;
    _bsp_icache_info.linesize = 16;
    _bsp_icache_info.ways = 2;

    /*
     * read sw1 on motherboard and invert sense of bits.
     */
    sw1 = ~ *(volatile unsigned char *)0x00304000;


    /*
     * Setup reload timer 2 to be a free running counter
     * that the network stack uses for timing.
     */
    init_rtimer();

#if 0
    {
	unsigned long now;
	unsigned long then;
	unsigned char pval = 0x20;
	
	fr30_io->ddra = 0x20;  /* set CS5 pin as port output */
	fr30_io->pdra = pval;  /* set initial value */
	now = then = _bsp_ms_ticks();
	while (1) {
	    now = _bsp_ms_ticks();
	    if (now != then) {
		then = now;
		pval ^= 0x20;
		fr30_io->pdra = pval;  /* set initial value */
	    }
	}
    }
#endif

    if (sw1 & 0x0f) {
	/* Use ethernet if we can find an IP address using BOOTP. */
	   
	addr[0] = 0x00;
	addr[1] = 0x00;
	addr[2] = 0x0E;
	addr[3] = 0x31;
	addr[4] = 0x00;
	addr[5] = sw1 & 0xf;

	eprocs = __enet_link_init(addr);
	if (eprocs) {
	    if (!_bsp_net_init(eprocs))
		bsp_printf("Network initialization failed.\n");
	}
	else
	    bsp_printf("Ethernet initialization failed.\n");
    }

    /*
     * Turn on RDY led.
     */
    *(unsigned char *)0x00303000 = 1;
}

