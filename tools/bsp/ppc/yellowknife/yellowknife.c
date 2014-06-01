/*
 * yka.c -- Support for Motorola Yellowknife platform.
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

#include <stdlib.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "queue.h"
#include "yellowknife.h"

#define UART_BUFSIZE 32

static bsp_queue_t uart1_queue;
static bsp_queue_t uart2_queue;
static char uart1_buffer[UART_BUFSIZE];
static char uart2_buffer[UART_BUFSIZE];

#if defined(__MPC106_MAP_A__)
#define io_base	  ((void*)PREP_IO_BASE)
#define	iack_base ((volatile int *)0xbffffff0)
#elif defined(__MPC106_MAP_B__)
#define io_base	  ((void*)CHRP_IO_BASE)
#define	iack_base ((volatile int *)0xfef00000)
#else
#error Must define Memory Map
#endif

static void *uart1_base;
static void *uart2_base;

#define uart1_irq 5
#define uart2_irq 4

#define __iorb(x) *((volatile unsigned char *)io_base + (x))
#define __iowb(x, n) *((volatile unsigned char *)io_base + (x))=(n)

/*
 * SuperIO defines.
 */
#define PNP_DEVSEL    0x07   /* device select register index */
#define PNP_ACTIVATE  0x30   /* device activation register index */

#define DEVID_UART1   6
#define DEVID_UART2   5

#define BASEADDR_MSB  0x60
#define BASEADDR_LSB  0x61

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

static int
uart_irq_disable(void *base)
{
    volatile unsigned char *tty_ien = base + SIO_IEN;
    unsigned char ien;

    ien = *tty_ien;
    *tty_ien = 0;

    return (ien & 5) != 0;
}

static void
uart_irq_enable(void *base)
{
    volatile unsigned char *tty_ien = base + SIO_IEN;

    *tty_ien = 5;
}


static void
uart_putchar(void *base, char ch)
{
    volatile unsigned char *tty_status;
    volatile unsigned char *tty_tx;

    tty_status = (volatile unsigned char *)base + SIO_LSTAT;
    tty_tx     = (volatile unsigned char *)base + SIO_TXD;

    while((*tty_status & SIO_LSTAT_TEMTY) == 0)
	;

    *tty_tx = ch;
}


static void
uart_write(void *base, const char *buf, int len)
{
    while (len-- > 0)
	uart_putchar(base, *buf++);
}


static int
uart_rcvchar(void *base)
{
    volatile unsigned char *tty_status;
    volatile unsigned char *tty_rx;
    unsigned char c;

    tty_status = (volatile unsigned char *)base + SIO_LSTAT;
    tty_rx     = (volatile unsigned char *)base + SIO_RXD;

    while((*tty_status & SIO_LSTAT_RRDY) == 0)
	;

    c = *tty_rx;

    return c;
}


static int
uart_getchar(void *base)
{
    int ch, was_enabled;

    was_enabled = uart_irq_disable(base);

    ch = _bsp_dequeue(base == uart1_base ? &uart1_queue : &uart2_queue);
    if (ch < 0)
	ch = uart_rcvchar(base);

    if (was_enabled)
	uart_irq_enable(base);
    return ch;
}


static int
uart_read(void *base, char *buf, int len)
{
    int ch;

    if (len <= 0)
	return 0;

    ch = uart_getchar(base);
    *buf = ch;
    return 1;
}


static volatile unsigned char *uart_dbg_base;

static int
uart_irq_handler(int irq_nr, void *regs)
{
    volatile unsigned char *tty_istat = uart_dbg_base + SIO_ISTAT;
    volatile unsigned char *tty_lstat = uart_dbg_base + SIO_LSTAT;
    int   ch;
    unsigned char istat, lstat;

    istat = *tty_istat; /* clear pending irq */
    istat &= 0x0f;

    if (istat == 4 || istat == 0x0c) {
	ch = uart_rcvchar((void *)uart_dbg_base);

	if (ch == '\003')
	    return 0;

	_bsp_enqueue(uart_dbg_base == uart1_base ? &uart1_queue : &uart2_queue,
		     ch);

    } else if (istat == 6) {
	lstat = *tty_lstat;
    } else if ((istat & 1) == 0)
	bsp_printf("unexpected sio interrupt: istat<0x%x>\n", istat);

    return 1;
}


static int uartA_baud, uartB_baud;

struct _baud {
    int baud;
    unsigned char msb;
    unsigned char lsb;
};

const static struct _baud bauds[] = {
    { 9600,  0, 12 },
    { 19200, 0,  6 },
    { 38400, 0,  3 },
    { 57600, 0,  2 }
};

static int
uart_setbaud(void *base, int bps)
{
    int   i;
    const struct _baud *bp;
    volatile unsigned char *lcr;
    
    for (i = 0, bp = bauds; i < (sizeof(bauds)/sizeof(bauds[0])); i++, bp++)
	if (bp->baud == bps) {
	    lcr = (volatile unsigned char *)base + SIO_LCTL;
	    *lcr = SIO_LCTL_DLATCH;

	    *((volatile unsigned char *)base + SIO_BAUDLO) = bp->lsb;
	    *((volatile unsigned char *)base + SIO_BAUDHI) = bp->msb;

	    *lcr = SIO_LCTL_8BIT | SIO_LCTL_1STOP | SIO_LCTL_NONE;

	    if (base == uart1_base)
		uartA_baud = bps;
	    else
		uartB_baud = bps;
	    return 0;
	}
    return -1;
}


static void
init_uart(unsigned char *base, int bps)
{
    volatile unsigned char dummy;

    uart_setbaud(base, bps);

    /* clear pending irq */
    dummy = *(volatile unsigned char *)(base + SIO_ISTAT);

    /* clear modem status */
    dummy = *(volatile unsigned char *)(base + SIO_MSTAT);

    /* enable FIFO */
    *(volatile unsigned char *)(base + SIO_FCTL) = 7;

    /* assert DTR and RTS, enable interrup line */
    *(volatile unsigned char *)(base + SIO_MCTL) = 0x0b;
}


static int
uart_control(void *base, int func, ...)
{
    int          i, retval = 0;
    va_list      ap;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    i = (base == uart1_base) ? uart1_irq : uart2_irq;

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(base, i);
	break;

      case COMMCTL_GETBAUD:
	retval = (base == uart1_base) ? uartA_baud : uartB_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = uart_irq_handler;
	uart_vec.next = NULL;
	bsp_install_vec(BSP_VEC_INTERRUPT, i,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(i);
	uart_irq_enable(base);
	uart_dbg_base = base;
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	uart_irq_disable(base);
	bsp_disable_irq(i);
	bsp_remove_vec(BSP_VEC_INTERRUPT, i, &uart_vec);
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
	{ NULL, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    },
    {
	{ "uart2", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ NULL, uart_write, uart_read,
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
#define _SIOW(i,x) __iowb(0x15c,(i)); __iowb(0x15d,(x))

    _bsp_queue_init(&uart1_queue, uart1_buffer, sizeof(uart1_buffer));
    _bsp_queue_init(&uart2_queue, uart2_buffer, sizeof(uart2_buffer));

    _SIOW(PNP_DEVSEL, DEVID_UART2);
    _SIOW(PNP_ACTIVATE, 0);
    _SIOW(BASEADDR_MSB,0x02);
    _SIOW(BASEADDR_LSB, 0xf8);
    _SIOW(PNP_ACTIVATE, 1);

    _SIOW(PNP_DEVSEL, DEVID_UART1);
    _SIOW(PNP_ACTIVATE, 0);
    _SIOW(BASEADDR_MSB,0x03);
    _SIOW(BASEADDR_LSB, 0xf8);
    _SIOW(PNP_ACTIVATE, 1);

    uart1_base = io_base + 0x3f8;
    uart2_base = io_base + 0x2f8;

    _bsp_comm_list[0].procs.ch_data = uart1_base;    
    _bsp_comm_list[1].procs.ch_data = uart2_base;    

    init_uart(uart1_base, 38400);
    init_uart(uart2_base, 38400);
}


/*
 *  Yellowknife uses the interrupt controller in the winbond part.
 */
#define PIC1_ICW1         0x20      /* Initialization write only */
#define PIC1_ICW2         0x21      /* Initialization write only */
#define PIC1_ICW3         0x21      /* Initialization write only (master) */
#define PIC1_ICW4         0x21      /* Initialization write only  */
#define PIC1_OCW1         0x21      /* Operation */
#define PIC1_OCW2         0x20      /* Operation write only */
#define PIC1_OCW3         0x20      /* Operation */

#define PIC2_ICW1         0xA0
#define PIC2_ICW2         0xA1
#define PIC2_ICW3         0xA1
#define PIC2_ICW4         0xA1  
#define PIC2_OCW1         0xA1
#define PIC2_OCW2         0xA0
#define PIC2_OCW3         0xA0

static bsp_vec_t *yk_vec_tbl[16];

static void	yk_ictrl_init(const struct bsp_irq_controller *ic);
static int	yk_ictrl_disable(const struct bsp_irq_controller *ic,
				 int irq_nr);
static void	yk_ictrl_enable(const struct bsp_irq_controller *ic,
				int irq_nr);

/* Yellowknife Interrupt controller. */
static const struct bsp_irq_controller yk_irq_controller = {
    1, 15,
    &yk_vec_tbl[0],
    yk_ictrl_init,
    yk_ictrl_disable,
    yk_ictrl_enable
};

/*
 *  ISR for external interrupts.
 */
static int
yk_irq_dispatch(int irq_nr, ex_regs_t *regs)
{
    bsp_vec_t	*vec;
    int		done = 0, isr, irq, imask;
    volatile int dummy;
    const struct bsp_irq_controller *ic = &yk_irq_controller;

    dummy = *iack_base;
    __eieio();
    
    __iowb(PIC1_OCW3, 0x0b);
    __eieio();
    isr = __iorb(PIC1_OCW3);

    if (isr == 4) {
	__iowb(PIC2_OCW3, 0x0b);
	__eieio();
	isr = __iorb(PIC2_OCW3);
	isr <<= 8;
    }

    if (isr) {
	for (imask = 1, irq = 0; irq < 16 && !(imask & isr); imask <<= 1, ++irq)
	    ;

	/* get head of vector list */
	vec = ic->vec_list[irq];

	while (!done && vec && vec->handler != NULL) {
	    done = vec->handler(irq, regs);
	    vec = vec->next;
	}

	/* now ack the 8259(s) */
	if (irq & 8) {
	    __iowb(PIC1_OCW2, 0x62);
	    __iowb(PIC2_OCW2, 0x20);
	} else {
	    __iowb(PIC1_OCW2, 0x20);
	}
    }

    return done;
}


/*
 *  Initialize interrupt controller.
 */
static void
yk_ictrl_init(const struct bsp_irq_controller *ic)
{
    int i;
    static bsp_vec_t yk_vec;

    for (i = 0; i <= (ic->last - ic->first); i++)
	ic->vec_list[i] = NULL;

    __iowb(PIC1_ICW1, 0x11);	/* need ICW4 */
    __iowb(PIC1_ICW2, 0x00);
    __iowb(PIC1_ICW3, 0x04);	/* IRQ2 has slave */
    __iowb(PIC1_ICW4, 0x01);	/* x86 mode */
    __iowb(PIC1_OCW1, 0xfb);	/* mask all but slave */

    __iowb(PIC2_ICW1, 0x11);	/* need ICW4 */
    __iowb(PIC2_ICW2, 0x08);	/* first IRQ is IRQ8 */
    __iowb(PIC2_ICW3, 0x02);	/* IRQ goes to master IRQ2 */
    __iowb(PIC2_ICW4, 0x01);	/* x86 mode */
    __iowb(PIC2_OCW1, 0xff);	/* all masked */

    yk_vec.handler = (void *)yk_irq_dispatch;
    yk_vec.next = NULL;
    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_CORE,
		    BSP_VEC_REPLACE, &yk_vec);
}


/*
 *  Disable an interrupt.
 */
static int
yk_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned char mask, port;

    if (irq_nr < 7)
	port = PIC1_OCW1;
    else {
	port = PIC2_OCW1;
	irq_nr -= 8;
    }
	
    mask = __iorb(port);

    if (mask & (1<<irq_nr))
	return 0; /* already disabled */

    mask |= (1<<irq_nr);

    __iowb(port, mask);    

    return 1;
}


/*
 *  Enable an interrupt.
 */
static void
yk_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned char mask, port;

    if (irq_nr < 7)
	port = PIC1_OCW1;
    else {
	port = PIC2_OCW1;
	irq_nr -= 8;
    }
	
    mask = __iorb(port);
    mask &= ~(1<<irq_nr);
    __iowb(port, mask);    
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
    _bsp_install_irq_controller(&yk_irq_controller);
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
    _bsp_platform_info.cpu = "PPC6xx";
    _bsp_platform_info.board = "Motorola Yellowknife";

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

    __decr_cnt = 2083; /* 33.330MHz clock */
}

