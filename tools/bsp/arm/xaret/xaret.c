/*
 * xaret.c -- Support for "xaret" Board
 *
 * Copyright (c) 2000 Red Hat, Inc.
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include <stdlib.h>
#include <bsp/xaret.h>
#include "queue.h"
#include "bsp_if.h"

/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)0,   (void *)0,   0, RAM_TOTAL_SIZE,   BSP_MEM_RAM   }
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = sizeof(_bsp_memory_list)/sizeof(_bsp_memory_list[0]);


#define UART_BASE 0x00600000
#define UART_BUFSIZE 32

static inline void __delay()
{
    int i = 5;
    while (--i >= 0)
	asm volatile ("mov  r0,r0" : /* no outputs */ : /* no inputs */  );
}

#define USE_DELAY 0

#if USE_DELAY
#define DELAY() __delay()
#else
#define DELAY()
#endif



#define IO_WRITE(reg,val) (*(volatile unsigned char *)(reg) = (val))
#define IO_READ(reg)      (*(volatile unsigned char *)(reg))


static bsp_queue_t uart_queue;
static char uart_buffer[UART_BUFSIZE];

/* register offsets */
#define __RXD		0	/* receive data, read, dlab = 0 */
#define __TXD		0	/* transmit data, write, dlab = 0 */
#define	__BAUDLO	0	/* baud divisor 0-7, read/write, dlab = 1 */
#define	__IEN		1	/* interrupt enable, read/write, dlab = 0 */
#define	__BAUDHI	1	/* baud divisor 8-15, read/write, dlab = 1 */
#define	__ISTAT		2	/* interrupt status, read, dlab = 0 */
#define	__FCTL		2	/* fifo control, write, dlab = 0 */
#define	__LCTL		3	/* line control, read/write */
#define	__MCTL		4	/* modem control read/write */
#define	__LSTAT		5	/* line status read */
#define	__MSTAT		6	/* modem status read */
#define	__SPR		7	/* scratch pad register */

/* line status register bit defines */
#define __LSTAT_RRDY	0x01	/* 1 = receive register ready */
#define __LSTAT_OVER	0x02	/* 1 = receive overrun error */
#define __LSTAT_PERR	0x04	/* 1 = receive parity error */
#define __LSTAT_FRAM	0x08	/* 1 = receive framing error */
#define __LSTAT_BRK	0x10	/* 1 = receive break */
#define __LSTAT_TRDY	0x20	/* 1 = transmit hold register empty */
#define __LSTAT_TEMTY	0x40	/* 1 = transmit register empty */
#define __LSTAT_ERR	0x80	/* 1 = any error condition */

#define __LCTL_8BIT   0x03
#define __LCTL_7BIT   0x02
#define __LCTL_1STOP  0x00
#define __LCTL_2STOP  0x04
#define __LCTL_NONE   0x00
#define __LCTL_ODD    0x08
#define __LCTL_EVEN   0x18
#define __LCTL_DLATCH 0x80

#define __MCTL_DTR    0x01
#define __MCTL_RTS    0x02
#define __MCTL_IEN    0x08


static void
uart_irq_enable(void *base)
{
#if 0 /* FIXME */
    IO_WRITE(base+__IEN, 5);
#endif
}

static int
uart_irq_disable(void *base)
{
    unsigned char ien;

    ien = IO_READ(base+__IEN);
    DELAY();
    IO_WRITE(base+__IEN, 0);
    DELAY();

    return (ien & 5) != 0;
}


static void
uart_putchar(void *base, char c)
{
    while((IO_READ(base+__LSTAT) & __LSTAT_TEMTY) == 0)
	DELAY();

    IO_WRITE(base+__TXD, c);
}


static void
uart_write(void *base, const char *buf, int len)
{
    while (len-- > 0)
	uart_putchar(base, *buf++);
}


static int
uart_rcvchar(void *base, int block)
{
    unsigned char ch;

    if (1 /* FIXME block */) {
	if ((IO_READ(base+__LSTAT) & __LSTAT_RRDY) == 0) {
	    IO_WRITE(base+__MCTL, __MCTL_RTS | __MCTL_DTR | __MCTL_IEN);
	    while((IO_READ(base+__LSTAT) & __LSTAT_RRDY) == 0)
		DELAY();
	}
    }

    if (IO_READ(base+__LSTAT) & __LSTAT_RRDY) {
	DELAY();
	ch = IO_READ(base+__RXD);
	IO_WRITE(base+__MCTL, __MCTL_IEN);
    } else
	return -1;

    return ch;
}


static int
uart_getchar(void *base)
{
    int ch, was_enabled;

    was_enabled = uart_irq_disable(base);
    ch = _bsp_dequeue(&uart_queue);
    if (ch < 0)
	ch = uart_rcvchar(base, 1);
    if (was_enabled)
	uart_irq_enable(base);
    return ch;
}


static int
uart_getchar_noblock(void *base)
{
    int ch, was_enabled;

    was_enabled = uart_irq_disable(base);
    ch = _bsp_dequeue(&uart_queue);
    if (ch < 0)
	ch = uart_rcvchar(base, 0);
    if (was_enabled)
	uart_irq_enable(base);
    return ch;
}


static int
uart_read(void *base, char *buf, int len)
{
    int ch, cnt;

    if (len <= 0)
	return 0;

    cnt = 0;
    while (cnt < len) {
	ch = uart_getchar_noblock(base);
	if (ch < 0)
	    break;
	*buf++ = ch;
	++cnt;
    }

    return cnt;
}

static void *uart_dbg_base;

static int
uart_irq_handler(int irq_nr, void *regs)
{
    unsigned char istat, lstat, ch;
    bsp_queue_t *qp = &uart_queue;

    istat = IO_READ(uart_dbg_base+__ISTAT) & 0x0f;
    DELAY();

    if (istat == 4 || istat == 0x0c) {
	ch = uart_rcvchar((void *)uart_dbg_base, 1);

	if (ch == '\003')
	    return 0;

	_bsp_enqueue(qp, ch);

    } else if (istat == 6) {
	lstat = IO_READ(uart_dbg_base+__LSTAT);
	DELAY();
    } else if ((istat & 1) == 0)
	bsp_printf("unexpected sio interrupt: istat<0x%x>\n", istat);
    
    return 0;
}


static int uart1_baud;

struct _baud {
    int baud;
    unsigned char msb;
    unsigned char lsb;
};

const static struct _baud bauds[] = {
    { 2400,  0, 48 },
    { 4800,  0, 24 },
    { 9600,  0, 12 },
    { 19200, 0,  6 },
    { 38400, 0,  3 }
};


static int
uart_setbaud(void *base, int bps)
{
    int   i;
    const struct _baud *bp;

    for (i = 0, bp = bauds; i < (sizeof(bauds)/sizeof(bauds[0])); i++, bp++)
	if (bp->baud == bps) {
	    IO_WRITE(base+__LCTL, __LCTL_DLATCH);
	    DELAY();
	    IO_WRITE(base+__BAUDLO, bp->lsb);
	    DELAY();
	    IO_WRITE(base+__BAUDHI, bp->msb);
	    DELAY();
	    IO_WRITE(base+__LCTL, __LCTL_8BIT | __LCTL_1STOP | __LCTL_NONE);
	    DELAY();
	    uart1_baud = bps;
	    return 0;
	}
    return -1;
}


static void
init_uart(unsigned char *base, int bps)
{
    uart_setbaud(base, bps);

    /* clear pending irq */
    (void)IO_READ(base+__ISTAT);
    DELAY();

    /* clear modem status */
    (void)IO_READ(base+__MSTAT);
    DELAY();

    /* enable FIFO */
    IO_WRITE(base+__FCTL, 7);
    DELAY();

    /* assert DTR and RTS, enable interrupt line */
    IO_WRITE(base+__MCTL, __MCTL_IEN | __MCTL_RTS | __MCTL_DTR);
    DELAY();
}


static int
uart_control(void *base, int func, ...)
{
    int          i, irq_nr, retval = 0;
    va_list      ap;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    irq_nr = 1;

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(base, i);
	break;

      case COMMCTL_GETBAUD:
	retval = uart1_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = uart_irq_handler;
	uart_vec.next = NULL;

	bsp_install_vec(BSP_VEC_INTERRUPT, irq_nr,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(irq_nr);

	uart_irq_enable(base);

	uart_dbg_base = base;
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	uart_irq_disable(base);
	bsp_disable_irq(irq_nr);
	bsp_remove_vec(BSP_VEC_INTERRUPT, irq_nr, &uart_vec);
	uart_dbg_base = NULL;
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = uart_irq_disable(base);
	break;

      case COMMCTL_IRQ_ENABLE:
	uart_irq_enable(base);
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
	{ "uart1", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE, uart_write, uart_read,
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
    _bsp_queue_init(&uart_queue, uart_buffer, sizeof(uart_buffer));

    init_uart((void*)UART_BASE, 19200);
}


/*
 * Set any board specific debug traps.
 */
void
_bsp_install_board_debug_traps(void)
{
}


/*
 * Install any board specific interrupt controllers.
 */
void
_bsp_install_board_irq_controllers(void)
{
}

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    /*
     * Override default platform info.
     */
    _bsp_platform_info.cpu = "SA-220";
    _bsp_platform_info.board = "XARET";
    _bsp_platform_info.extra = "ARM is a Registered Trademark of Advanced RISC Machines Limited.\nOther Brands and Trademarks are the property of their respective owners.";
}


#if 0
unsigned long
_bsp_ms_ticks()
{
    static unsigned long cnt = 0;

    return cnt++;
}
#endif

