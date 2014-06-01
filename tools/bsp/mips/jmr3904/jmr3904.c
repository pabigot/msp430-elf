/*
 * jmr3904.c -- Support for JMR3904 board.
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
#include "queue.h"
#include "bsp_if.h"
#include "net.h"

#define UART_BUFSIZE 64
static bsp_queue_t uart1_queue;
static char uart1_buffer[UART_BUFSIZE];


typedef struct {
    volatile unsigned short lcr;
    volatile unsigned short pad1;
#define LCR_8_BIT    0x00
#define LCR_7_BIT    0x01
#define LCR_1_STOP   0x00
#define LCR_2_STOP   0x04
#define LCR_PAR_NONE 0x00
#define LCR_PAR_ODD  0x08
#define LCR_PAR_EVEN 0x18
#define LCR_CLK_T0   0x00
#define LCR_CLK_BRG  0x20
#define LCR_CLK_SCLK 0x40
#define LCR_HSE      0x100
    volatile unsigned short lsr;
    volatile unsigned short pad2;
#define LSR_UOER 2
#define LSR_UPER 4
#define LSR_UFER 8
    volatile unsigned short dicr;
    volatile unsigned short pad3;
#define DICR_RDIE  1
#define DICR_TDIE  2
#define DICR_ERIE  4
#define DICR_SDMAE 8
    volatile unsigned short disr;
    volatile unsigned short pad4;
#define DISR_RDIS  1
#define DISR_TDIS  2
#define DISR_ERIS  4
#define DISR_SDMA  8
    volatile unsigned short fcr;
    volatile unsigned short pad5;
#define FCR_FRSTE  1
#define FCR_RFRST  2
#define FCR_TFRST  4
#define FCR_TDL    8
#define FCR_RDL   16
    volatile unsigned short brg;
    volatile unsigned short pad6;
#define BRG_T0   0x0000
#define BRG_T2   0x0100
#define BRG_T4   0x0200
#define BRG_T5   0x0300
    volatile unsigned int   pad7[2];

    volatile unsigned char  txd;
    volatile unsigned char  pad8[15];

    volatile unsigned char  rxd;
    volatile unsigned char  pad9[3];
} r3904_sio_t;

#define UART_CH_A  ((r3904_sio_t *)0xfffff300)


static int led_shadow;

void
led_on(val)
{
    led_shadow |= val;
    *(volatile unsigned char *)0xb2500000 = led_shadow;
}

void
led_off(val)
{
    led_shadow &= ~val;
    *(volatile unsigned char *)0xb2500000 = led_shadow;
}

static void
uart_irq_enable(r3904_sio_t *chan)
{
    chan->dicr = DICR_RDIE;
}

static int
uart_irq_disable(r3904_sio_t *chan)
{
    int was_enabled;

    was_enabled = (chan->dicr & DICR_RDIE) != 0;
    chan->dicr = 0;
    return was_enabled;
}


static void
uart_putchar(void *chan_data, char ch)
{
    r3904_sio_t    *chan = chan_data;
    unsigned short disr;
    int was_enabled;

    was_enabled = uart_irq_disable(chan);
    while (!((disr = chan->disr) & DISR_TDIS))
	;

    chan->txd = ch;
    chan->disr = DISR_RDIS;

    if (was_enabled)
	uart_irq_enable(chan);
}


static void
uart_write(void *chan_data, const char *buf, int len)
{
    while (len-- > 0)
	uart_putchar(chan_data, *buf++);
}


static int
uart_rcvchar(void *chan_data)
{
    r3904_sio_t    *chan = chan_data;
    unsigned short disr;
    int            ch;

    while (!((disr = chan->disr) & DISR_RDIS))
	;

    ch = chan->rxd;
    chan->disr = DISR_TDIS;

    return ch;
}

static int
uart_getchar(void *chan_data)
{
    r3904_sio_t *chan = chan_data;
    int ch, was_enabled;

    was_enabled = uart_irq_disable(chan);
    ch = _bsp_dequeue(&uart1_queue);
    if (ch < 0)
	ch = uart_rcvchar(chan);
    if (was_enabled)
	uart_irq_enable(chan);
    return ch;
}


static int
uart_read(void *chan_data, char *buf, int len)
{
    int ch;

    if (len <= 0)
	return 0;

    ch = uart_getchar(chan_data);
    *buf = ch;
    return 1;
}


static int
uart_irq_handler(int irq_nr, void *regs)
{
    unsigned char ch;

    if (UART_CH_A->disr & DISR_RDIS) {
	ch = uart_rcvchar(UART_CH_A);
	if (ch == '\003') {
	    /* make sure nothing is in the queue */
	    while (_bsp_dequeue(&uart1_queue) >= 0)
		;
	    return 0;
	}
	_bsp_enqueue(&uart1_queue, ch);
    }
    return 1;
}

struct _baud {
    int  baud;
    int  val;
};

static const struct _baud *_bauds;

static const struct _baud _bauds_50[] = {
    {  9600, BRG_T2 | 20 },
    { 19200, BRG_T2 | 10 },
    { 38400, BRG_T2 |  5 },
    { 0, 0 }
};

static const struct _baud _bauds_66[] = {
    {  9600, BRG_T2 | 27 },
    { 19200, BRG_T0 | 54 },
    { 38400, BRG_T0 | 27 },
    { 0, 0 }
};

static int uartA_baud;

static int
uart_setbaud(r3904_sio_t *chan, int bps)
{
    int i;

    for (i = 0; _bauds[i].baud; i++) {
	if (_bauds[i].baud == bps) {
	    UART_CH_A->brg = _bauds[i].val;
	    if (chan == UART_CH_A)
		uartA_baud = bps;
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
    r3904_sio_t *chan = chan_data;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(chan, i);
	break;

      case COMMCTL_GETBAUD:
	retval = uartA_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = uart_irq_handler;
	uart_vec.next = NULL;

	bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SIO0,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(BSP_IRQ_SIO0);

	uart_irq_enable(chan);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	uart_irq_disable(chan);
	bsp_disable_irq(BSP_IRQ_SIO0);
	bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SIO0, &uart_vec);
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
    r3904_sio_t *chan;

    chan = UART_CH_A;

    chan->lcr  = LCR_CLK_BRG | LCR_PAR_NONE | LCR_1_STOP | LCR_8_BIT;
    chan->dicr = 0;
    chan->fcr = 7;
    chan->fcr = 0;

    uart_setbaud(chan, bps);
}


struct bsp_comm_channel _bsp_comm_list[] = 
{
    {
	{ "uart1", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_CH_A, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
};


int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);


/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    volatile int dummy;

    /*
     * Override default platform info.
     */
    if (*(unsigned *)(_bsp_ram_info_ptr + 4)) {
	_bsp_platform_info.cpu = "TX3904F-66";
	_bauds = _bauds_66;
    } else {
	_bsp_platform_info.cpu = "TX3904F-50";
	_bauds = _bauds_50;
    }

    _bsp_queue_init(&uart1_queue, uart1_buffer, sizeof(uart1_buffer));
    init_uart(0, 38400);

    dummy = UART_CH_A->rxd;
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
    { (void *)0, (void *)0x80000000, 0, 0, BSP_MEM_RAM },
#if defined(BOOT_FROM_FLASH)
    { (void *)0x1fc00000, (void *)0xbfc00000, 0, 4 * 1024 * 1024, BSP_MEM_FLASH }
#else
    { (void *)0x1f000000, (void *)0xbf000000, 0, 4 * 1024 * 1024, BSP_MEM_FLASH }
#endif
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = 2;

/* initialize a millisecond timer. i == timer number (0-2) */
extern void init_ms_timer(int i);

extern void (*_bsp_kill_hook)(void);
extern void _bsp_install_vsr(void);

static void
kill_hook(void)
{
    int i;
    int  cur_port;
    struct bsp_comm_channel *com;

    __dcache_flush(0,-1);
    __icache_flush(0,-1);
    _bsp_install_vsr();
    init_ms_timer(1);
    (void)_bsp_ms_ticks();

    /* the app may have left some interrupts on */
    for (i = BSP_IRQ_SW0; i <= BSP_IRQ_TMR2; i++)
	bsp_disable_irq(i);

    cur_port = bsp_set_debug_comm(-1);
    if (cur_port == 0)
	com = &_bsp_comm_list[0];
    else
	com = _bsp_net_channel;

    /* reenable debug channel interrupts */
    (*com->procs.__control)(com->procs.ch_data, COMMCTL_REMOVE_DBG_ISR);
    (*com->procs.__control)(com->procs.ch_data, COMMCTL_INSTALL_DBG_ISR);
    bsp_set_console_comm(cur_port);   
}


static int
mem_validate(void *addr, int len)
{
    /* access to kuseg appears to hang */
    if (((unsigned long)addr & 0x80000000) == 0)
	return 0;
    addr += (len - 1);
    if (((unsigned long)addr & 0x80000000) == 0)
	return 0;
    return 1;
}


/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    extern struct bsp_comm_procs *__enet_link_init(void);
    struct bsp_comm_procs *eprocs;
    unsigned char sw2;

    /*
     * Finish setup of RAM description. Early initialization put a
     * pointer to SRAM size and clock flag in _bsp_ram_info_ptr.
     */
    _bsp_memory_list[0].nbytes = *(unsigned *)_bsp_ram_info_ptr;

    _bsp_platform_info.board = "JMR-TX3904";

    /*
     * Override cache info.
     */
    _bsp_dcache_info.size = 1024;
    _bsp_dcache_info.linesize = 4;
    _bsp_dcache_info.ways = 2;

    _bsp_icache_info.size = 4096;
    _bsp_icache_info.linesize = 16;
    _bsp_icache_info.ways = 1;

    _bsp_kill_hook = kill_hook;
    init_ms_timer(1);
    (void)_bsp_ms_ticks();

    __mem_read_validate_hook = mem_validate;
    __mem_write_validate_hook = mem_validate;

    sw2 = *(unsigned char *)0xb2500000;
    if (sw2 & 0x10) {
	eprocs = __enet_link_init();
	if (eprocs) {
	    if (!_bsp_net_init(eprocs))
		bsp_printf("Network initialization failed.\n");
	} else
	    bsp_printf("Ethernet initialization failed.\n");
    }
}

