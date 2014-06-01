/*
 * evb403.c -- IBM EVB 403 initialization.
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

#define SPU_ADDR ((uart_403_t *)0x40000000)

#define UART_BUFSIZE 32

/*---------------------------------------------------------------------------
 * EVB403 board specific 16550 code.
 ---------------------------------------------------------------------------*/

#define SIO_BASE 		((volatile unsigned char *)(0x7e000000))

/* register offsets */
#define SIO_RXD			0	/* receive data, read, dlab = 0 */
#define SIO_TXD			0	/* transmit data, write, dlab = 0 */
#define	SIO_BAUDLO		0	/* baud divisor, rd/wr, dlab = 1 */
#define	SIO_IEN			1	/* interrupt enable, rd/wr, dlab = 0 */
#define	SIO_BAUDHI		1	/* baud divisor, rd/wr, dlab = 1 */
#define	SIO_ISTAT		2	/* interrupt status, read, dlab = 0 */
#define	SIO_FCTL		2	/* fifo control, write, dlab = 0 */
#define	SIO_LCTL		3	/* line control, read/write */
#define	SIO_MCTL		4	/* modem control read/write */
#define	SIO_LSTAT		5	/* line status read */
#define	SIO_MSTAT		6	/* modem status read */
#define	SIO_SPR			7	/* scratch pad register */

/* line status register bit defines */
#define SIO_LSTAT_RRDY	0x01	/* 1 = receive register ready */
#define SIO_LSTAT_OVER	0x02	/* 1 = receive overrun error */
#define SIO_LSTAT_PERR	0x04	/* 1 = receive parity error */
#define SIO_LSTAT_FRAM	0x08	/* 1 = receive framing error */
#define SIO_LSTAT_BRK	0x10	/* 1 = receive break */
#define SIO_LSTAT_TRDY	0x20	/* 1 = transmit hold register empty */
#define SIO_LSTAT_TEMTY	0x40	/* 1 = transmit register empty */
#define SIO_LSTAT_ERR	0x80	/* 1 = any error condition */

#define SIO_LCTL_8BIT   0x03
#define SIO_LCTL_7BIT   0x02
#define SIO_LCTL_1STOP  0x00
#define SIO_LCTL_2STOP  0x04
#define SIO_LCTL_NONE   0x00
#define SIO_LCTL_ODD    0x08
#define SIO_LCTL_EVEN   0x18
#define SIO_LCTL_DLATCH 0x80


static bsp_queue_t uart_queue;
static char uart_buffer[UART_BUFSIZE];

static bsp_queue_t sio_queue;
static char sio_buffer[UART_BUFSIZE];

static void
uart_enable_irq(uart_403_t *uart)
{
    uart->sprc |= UART_RC_RXINT;
}

static int
uart_disable_irq(uart_403_t *uart)
{
    unsigned char rc;

    rc = uart->sprc;
    uart->sprc = rc & ~UART_RC_RXINT;
    __eieio();

    return (rc & UART_RC_RXINT) == UART_RC_RXINT;
}

static void
uart_putchar(void *which, char ch)
{
    uart_403_t *uart = (uart_403_t *)which;

    while (!(uart->spls & UART_LS_TXRDY))
	if (uart->sphs & (UART_HS_DSR | UART_HS_CTS)) {
	    uart->sphs = (UART_HS_DSR | UART_HS_CTS);
	    __eieio();
	}

    uart->spb = ch;
    __eieio();
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
    uart_403_t *uart = (uart_403_t *)which;
    unsigned char ch;

    while (!(uart->spls & UART_LS_RXRDY))
	if (uart->spls & (UART_LS_FE | UART_LS_OE | UART_LS_LB | UART_LS_PE)) {
#if 1
	    bsp_printf("uart_rcvchar: error[0x%x]\n", uart->spls);
#endif
	    uart->spls = (UART_LS_FE | UART_LS_OE | UART_LS_LB | UART_LS_PE);
	    __eieio();
	}

    ch = uart->spb;

    return ch;
}


static int
uart_getchar(void *which)
{
    uart_403_t *uart = (uart_403_t *)which;
    int was_enabled;
    int ch;

    was_enabled = uart_disable_irq(uart);
    ch = _bsp_dequeue(&uart_queue);
    if (ch < 0)
	ch = uart_rcvchar(uart);
    if (was_enabled)
	uart_enable_irq(uart);
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


struct baud {
    int baud;
    unsigned char h;
    unsigned char l;
};

const static struct baud uart_bauds[] = {
    {  9600, 0, 47 },
    { 19200, 0, 23 },
    { 38400, 0, 11 },
    { 57600, 0,  7 }
};

#define NUM_UART_BAUDS (sizeof(uart_bauds)/sizeof(uart_bauds[0]))

static int uart_baud;

static int
uart_setbaud(void *which, int bps)
{
    uart_403_t *uart = (uart_403_t *)which;
    int   i;
    const struct baud *bp;

    for (i = 0, bp = uart_bauds; i < NUM_UART_BAUDS; i++, bp++)
	if (bp->baud == bps) {
	    uart->brdh = bp->h;
	    __eieio();
	    uart->brdl = bp->l;
	    __eieio();
	    uart_baud = bps;
	    return 0;
	}
    return -1;
}


static void
init_uart(int bps)
{
    uart_403_t *uart = SPU_ADDR;
    unsigned iocr;

    iocr = __get_iocr();
    /*
     * Use SysClk for baud rate generator.
     * Config DSR/DTR.
     */
    iocr |= 2;

    /*
     * High level from uart indicates interrupt.
     */
    iocr |= 0x10000000;

    __set_iocr(iocr);

    

    uart_setbaud(uart, bps);

    uart->spctl = UART_CTL_NORMAL_MODE | UART_CTL_8BIT |
	          UART_CTL_NONE | UART_CTL_1STOP |
	          UART_CTL_DTR | UART_CTL_RTS;
    __eieio();

    uart->sprc = UART_RC_ENABLE;
    __eieio();

    uart->sptc = UART_TC_ENABLE;
    __eieio();

    uart->sphs = UART_HS_DSR | UART_HS_CTS;
    __eieio();

    uart->spls = UART_LS_FE | UART_LS_OE | UART_LS_PE | UART_LS_LB;
    __eieio();
}


static int
uart_irq_handler(int irq_nr, void *regs)
{
    uart_403_t *uart = SPU_ADDR;
    unsigned char ch;

    if (uart->spls & UART_LS_RXRDY) {
	ch = uart_rcvchar(uart);
	if (ch == '\003') {
	    return 0;
	}
	_bsp_enqueue(&uart_queue, ch);
    }
    return 1;
}


static void
sio_enable_irq(volatile unsigned char *base)
{
    volatile unsigned char *sio_ien = base + SIO_IEN;

    *sio_ien = 5;
    __eieio();
}

static int
sio_disable_irq(volatile unsigned char *base)
{
    volatile unsigned char *sio_ien = base + SIO_IEN;
    unsigned char ien;

    ien = *sio_ien;
    __eieio();
    *sio_ien = 0;
    __eieio();

    return (ien & 5) != 0;
}

static void
sio_putchar(void *which, char ch)
{
    volatile unsigned char *tty_status;
    volatile unsigned char *tty_tx;

    tty_status = (volatile unsigned char *)which + SIO_LSTAT;
    tty_tx     = (volatile unsigned char *)which + SIO_TXD;

    while((*tty_status & SIO_LSTAT_TEMTY) == 0)
	;

    *tty_tx = ch;
    __eieio();
}


static void
sio_write(void *which, const char *buf, int len)
{
    while (len-- > 0)
	sio_putchar(which, *buf++);
}

static int
sio_rcvchar(void *which)
{
    volatile unsigned char *tty_status;
    volatile unsigned char *tty_rx;
    unsigned char c;


    tty_status = (volatile unsigned char *)which + SIO_LSTAT;
    tty_rx     = (volatile unsigned char *)which + SIO_RXD;

    while((*tty_status & SIO_LSTAT_RRDY) == 0)
	;

    c = *tty_rx;
    __eieio();

    return c;
}


static int
sio_getchar(void *which)
{
    int was_enabled;
    int ch;

    was_enabled = sio_disable_irq(which);
    ch = _bsp_dequeue(&sio_queue);
    if (ch < 0)
	ch = sio_rcvchar(which);
    if (was_enabled)
	sio_enable_irq(which);
    return ch;
}

static int
sio_read(void *which, char *buf, int len)
{
    int ch;

    if (len <= 0)
	return 0;

    ch = sio_getchar(which);
    *buf = ch;
    return 1;
}


const static struct baud sio_bauds[] = {
    {   9600, 0, 48 },
    {  19200, 0, 24 },
    {  38400, 0, 12 },
    {  57600, 0,  8 },
    { 115200, 0,  4 }
};

#define NUM_SIO_BAUDS (sizeof(sio_bauds)/sizeof(sio_bauds[0]))

static int sio_baud;

static int
sio_setbaud(void *which, int bps)
{
    volatile unsigned char *lcr;
    int   i;
    const struct baud *bp;

    for (i = 0, bp = sio_bauds; i < NUM_SIO_BAUDS; i++, bp++)
	if (bp->baud == bps) {
	    lcr = SIO_BASE + SIO_LCTL;
	    *lcr = SIO_LCTL_DLATCH;
	    __eieio();
	    *(SIO_BASE + SIO_BAUDLO) = bp->l;
	    __eieio();
	    *(SIO_BASE + SIO_BAUDHI) = bp->h;
	    __eieio();
	    *lcr = SIO_LCTL_8BIT | SIO_LCTL_1STOP | SIO_LCTL_NONE;
	    __eieio();
	    sio_baud = bps;
	    return 0;
	}
    return -1;
}


static void
init_sio(int bps)
{
    volatile unsigned char dummy;

    sio_setbaud((void *)SIO_BASE, bps);

    /* clear pending irq */
    dummy = *(SIO_BASE + SIO_ISTAT);
    __eieio();

    /* clear modem status */
    dummy = *(SIO_BASE + SIO_MSTAT);
    __eieio();

    /* enable fifo */
    *(SIO_BASE + SIO_FCTL) = 7;
    __eieio();

    /* assert DTR and RTS */
    *(SIO_BASE + SIO_MCTL) = 3;
    __eieio();
}


static int
sio_irq_handler(int irq_nr, void *regs)
{
    volatile unsigned char *tty_istat = SIO_BASE + SIO_ISTAT;
    volatile unsigned char *tty_lstat = SIO_BASE + SIO_LSTAT;
    int   ch;
    unsigned char istat, lstat;

    istat = *tty_istat; /* clear pending irq */
    istat &= 0x0f;

    if (istat == 4 || istat == 0x0c) {
	ch = sio_rcvchar((void *)SIO_BASE);

	if (ch == '\003')
	    return 0;

	_bsp_enqueue(&sio_queue, ch);

    } else if (istat == 6) {
	lstat = *tty_lstat;
    } else if ((istat & 1) == 0)
	bsp_printf("unexpected sio interrupt: istat<0x%x>\n", istat);

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
	if (which == ((void *)SIO_BASE))
	    retval = sio_setbaud(which, i);
	else
	    retval = uart_setbaud(which, i);
	break;

      case COMMCTL_GETBAUD:
	if (which == ((void *)SIO_BASE))
	    retval = sio_baud;
	else
	    retval = uart_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.next = NULL;
	if (which == ((void *)SIO_BASE)) {
	    uart_vec.handler = sio_irq_handler;
	    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_EXT1,
			    BSP_VEC_REPLACE, &uart_vec);
	    bsp_enable_irq(BSP_IRQ_EXT1);
	    sio_enable_irq(SIO_BASE);
	} else {
	    uart_vec.handler = uart_irq_handler;
	    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SERIAL_RX,
			    BSP_VEC_REPLACE, &uart_vec);
	    bsp_enable_irq(BSP_IRQ_SERIAL_RX);
	    uart_enable_irq(SPU_ADDR);
	}
	return 0;
	
      case COMMCTL_REMOVE_DBG_ISR:
	if (which == ((void *)SIO_BASE)) {
	    sio_disable_irq(SIO_BASE);
	    bsp_disable_irq(BSP_IRQ_EXT1);
	    bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_EXT1, &uart_vec);
	} else {
	    uart_disable_irq(SPU_ADDR);
	    bsp_disable_irq(BSP_IRQ_SERIAL_RX);
	    bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SERIAL_RX, &uart_vec);
	}
	return 0;

      case COMMCTL_IRQ_DISABLE:
	if (which == ((void *)SIO_BASE))
	    retval = sio_disable_irq(SIO_BASE);
	else
	    retval = uart_disable_irq(SPU_ADDR);
	break;

      case COMMCTL_IRQ_ENABLE:
	if (which == ((void *)SIO_BASE))
	    sio_enable_irq(SIO_BASE);
	else
	    uart_enable_irq(SPU_ADDR);
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
	{ "serial2", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void *)SIO_BASE, sio_write, sio_read,
	  sio_putchar, sio_getchar, uart_control }
    },
    {
	{ "serial1", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ SPU_ADDR, uart_write, uart_read,
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
    _bsp_queue_init(&sio_queue, sio_buffer, sizeof(sio_buffer));
    init_sio(115200);
    _bsp_queue_init(&uart_queue, uart_buffer, sizeof(uart_buffer));
    init_uart(38400);
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
    _bsp_platform_info.cpu = "PPC403GA";
    _bsp_platform_info.board = "IBM Eval Board";

    /*
     * Override cache info.
     */
    _bsp_dcache_info.size = 1024;
    _bsp_dcache_info.linesize = 16;
    _bsp_dcache_info.ways = 2;

    _bsp_icache_info.size = 2048;
    _bsp_icache_info.linesize = 16;
    _bsp_icache_info.ways = 2;

    /*
     * Enable caching for DRAM and FLASH
     */
    __set_dccr(0x80000001);
    __set_iccr(0x80000001);

    __decr_cnt = 3000;  /* !!!FIXME!!! */
}

