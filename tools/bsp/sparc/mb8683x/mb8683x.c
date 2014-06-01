/*
 * mb.c -- Support for Fujitsu MB86831 Motherboard.
 *         This motherboard supports a number of 8683x CPU boards.
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
#include "net.h"
#include "queue.h"
#include "mb8683x.h"
#include "mb86940.h"

#define UART_0  ((void *)0)
#define UART_1  ((void *)1)

#define UART_BUFSIZE 32

static bsp_queue_t uart1_queue;
static char uart1_buffer[UART_BUFSIZE];
static bsp_queue_t uart2_queue;
static char uart2_buffer[UART_BUFSIZE];


static void
uart_putchar(void *base, char c)
{
    int  reg;

    reg = (base == UART_0) ? MB86940_SERDAT0 : MB86940_SERDAT1;

    while (!(mb86940_read(reg+1) & SER_STAT_TXRDY))
	;

    mb86940_write(reg, c);

#if 1
    while (!(mb86940_read(reg+1) & SER_STAT_TXRDY))
	;
#endif
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
    int  reg;

    reg = (base == UART_0) ? MB86940_SERDAT0 : MB86940_SERDAT1;

    while (!(mb86940_read(reg+1) & SER_STAT_RXRDY))
	;

    return mb86940_read(reg);
}


static int
uart_getchar(void *base)
{
    int		ch, enabled, irq;
    bsp_queue_t *qp;

    if (base == UART_0) {
	qp = &uart1_queue;
	irq = BSP_EXC_INT10;
    } else {
	qp = &uart2_queue;
	irq = BSP_EXC_INT7;
    }

    enabled = bsp_disable_irq(irq);
    ch = _bsp_dequeue(qp);
    if (ch < 0)
	ch = uart_rcvchar(base);
    if (enabled)
	bsp_enable_irq(irq);
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


static void *uart_dbg_base;

static int
uart_irq_handler(int irq_nr, void *regs)
{
    int  ch, reg, nr, rval = 1;
    bsp_queue_t *qp;

    if (uart_dbg_base == UART_0) {
	qp = &uart1_queue;
	reg = MB86940_SERDAT0;
	nr = 10;
    } else {
	qp = &uart2_queue;
	reg = MB86940_SERDAT1;
	nr = 7;
    }

    if (mb86940_read(reg+1) & SER_STAT_RXRDY) {
	ch = mb86940_read(reg);
	if (ch == '\003') {
	    rval = 0;
	    /* make sure nothing is in the queue */
	    while (_bsp_dequeue(qp) >= 0)
		;
	} else
	    _bsp_enqueue(qp, ch);
    }

    mb86940_write(MB86940_IRQCLR, (1 << nr));
    mb86940_write(MB86940_IRLATCH, 0x10);

    return rval;
}


/*
 * Both UART channels share the same baud rate clock.
 */
static int cur_baud;
static int
uart_setbaud(void *base, int bps)
{
    unsigned int clk;
    unsigned int tval;

    switch (bps) {
      case 19200:
      case 9600:
      case 4800:
      case 2400:
      case 1200:
      case 600:
      case 300:
	break;
      default:
	return -1;
    }

    cur_baud = bps;
    
    clk = *(unsigned char *)CLKSW_ADDR;
    if (clk & 0x80)
	clk = 10;

    clk = (clk & 0x3f) * 1000000;  /* in MHz */

    tval = clk / bps;
    tval /= 32;
    tval -= 1;

    mb86940_write(MB86940_TCR3, TCR_CE|TCR_CLKINT|TCR_OUTC3|TCR_SQWAVE);

    mb86940_write(MB86940_RELOAD3, tval);

    return 0;
}


static void
init_uart(void *base, int bps)
{
    int  ser_cmd;
    int  i;
#define DELAY(x)   for (i = 0; i < x; i++)

    uart_setbaud(base, bps);

    ser_cmd = (base == UART_0) ? MB86940_SERCMD0 : MB86940_SERCMD1;

    mb86940_write(ser_cmd, 0);
    DELAY(100);
    mb86940_write(ser_cmd, 0);
    DELAY(100);
    mb86940_write(ser_cmd, 0);
    DELAY(100);
     
    mb86940_write(ser_cmd, SER_CMD_IRST);
    DELAY(100);

    /* first write after reset is to mode register */
    mb86940_write(ser_cmd, SER_DIV16_CLK|SER_8BITS|SER_NO_PARITY|SER_STOP1);
    DELAY(100);

    /* subsequent writes are to command register */
    mb86940_write(ser_cmd, SER_CMD_RTS|SER_CMD_DTR|SER_CMD_EFR|\
		  SER_CMD_RXEN|SER_CMD_TXEN);
    DELAY(100);
}


static int
uart_control(void *base, int func, ...)
{
    int          i, exc_nr, retval = 0;
    va_list      ap;
    static bsp_vec_t uart_vec;

    exc_nr = (base == UART_0) ? BSP_EXC_INT10 : BSP_EXC_INT7;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(base, i);
	break;

      case COMMCTL_GETBAUD:
	retval = cur_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = uart_irq_handler;
	uart_vec.next = NULL;
	bsp_install_vec(BSP_VEC_EXCEPTION, exc_nr,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(exc_nr);
	uart_dbg_base = base;
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	bsp_disable_irq(exc_nr);
	bsp_remove_vec(BSP_VEC_EXCEPTION, exc_nr, &uart_vec);
	uart_dbg_base = NULL;
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = bsp_disable_irq(exc_nr);
	break;

      case COMMCTL_IRQ_ENABLE:
	bsp_enable_irq(exc_nr);
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
	{ UART_0, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    },
    {
	{ "uart2", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ UART_1, uart_write, uart_read,
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
#ifdef BSP_LOG
    unsigned char sw1;
    extern int bsp_dump_flag;


    sw1 = ~(*(unsigned char *)SW1_ADDR);
    if (sw1 & 0x20)
	bsp_dump_flag = 1;
#endif

    _bsp_queue_init(&uart1_queue, uart1_buffer, sizeof(uart1_buffer));
    _bsp_queue_init(&uart2_queue, uart2_buffer, sizeof(uart2_buffer));

    init_uart(UART_0, 19200);
    init_uart(UART_1, 19200);
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
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)0x04000000, (void *)0x04000000, 0, 0, BSP_MEM_RAM }
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = 1;



static unsigned long ticks;
static unsigned short last_count;


unsigned long
_bsp_ms_ticks(void)
{
    unsigned short current;
    unsigned short diff;

    current = mb86940_read(MB86940_CNT1);

    /* its a down counter, so use "last - current" */
    diff = last_count - current;
    ticks += diff;
    last_count = current;

    return ticks;
}


static void
init_ms_timer(void)
{
    unsigned int clk;

    /* make sure its disabled */
    mb86940_write(MB86940_TCR1, 0);

    clk = *(unsigned char *)CLKSW_ADDR;
    if (clk & 0x80)
	clk = 10;

    /* could be 10 - 66 MHz */
    clk = (clk & 0x3f) * 1000000;  /* in Hz */

    /*
     * Timer internal clock is divide by two.
     * Prescaler divides by 128
     */
    clk = clk / 256;

    clk = clk / 1000;
    if (clk > 255)
	clk = 255;

    mb86940_write(MB86940_PRS1, PRS_ODIV128 | clk);
    mb86940_write(MB86940_TCR1, TCR_CE|TCR_CLKPRS|TCR_OUTC3|TCR_SQWAVE);
    mb86940_write(MB86940_RELOAD1, 0xffff);
}


static inline unsigned int
get_ver2(void)
{
    unsigned int rval, *p;
    
    p = (unsigned int *)0x00020000;

    asm volatile("lda [%1] 1, %0\n" : "=r" (rval) : "r" (p));

    return rval;
}


extern void (*_bsp_kill_hook)(void);
extern void _bsp_install_vsr(void);

static void
kill_hook(void)
{
    /* mask individual interrupts, unmask IRL */
    mb86940_write(MB86940_IRQMASK, 0x7ffe);

    /* setup triggers for interrupts 15-8 */
    mb86940_write(MB86940_TRIG0,
		  (TRIG_EDGE_H  << 14) | (TRIG_LEVEL_L << 12) |
		  (TRIG_LEVEL_H << 10) | (TRIG_LEVEL_L <<  8) |
		  (TRIG_LEVEL_L <<  6) | (TRIG_LEVEL_H <<  4) |
		  (TRIG_LEVEL_H <<  2));

    /* setup triggers for interrupts 7-1 */
    mb86940_write(MB86940_TRIG1,
		  (TRIG_LEVEL_H << 14) | (TRIG_LEVEL_H << 12) |
		  (TRIG_LEVEL_L << 10) | (TRIG_LEVEL_L <<  8) |
		  (TRIG_LEVEL_H <<  6) | (TRIG_LEVEL_L <<  4));

    /* clear any sensed interrupts */
    mb86940_write(MB86940_IRQCLR, 0xfffe);

    /* clear IRL */
    mb86940_write(MB86940_IRLATCH, 0x10);

    _bsp_install_vsr();
    bsp_debug_irq_enable();
    bsp_enable_irq(BSP_EXC_INT15); /* NMI */
}


extern struct bsp_comm_procs *__enet_link_init(enet_addr_t enet_addr);

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    enet_addr_t addr;
    unsigned char sw1;
    struct bsp_comm_procs *eprocs;
    unsigned int cpu_ver;

    /*
     * Finish setup of RAM description. Early initialization put a
     * physical pointer to the top of RAM in _bsp_ram_info_ptr.
     */
    _bsp_memory_list[0].nbytes = (long)_bsp_ram_info_ptr;

    cpu_ver = get_ver2() & 0xffff;

    /*
     * Override default platform info.
     */
    _bsp_platform_info.board = "Fujitsu MB8683X EVB";

    switch (cpu_ver) {
      case 0:
	_bsp_platform_info.cpu = "MB86831";
	_bsp_dcache_info.size = 2048;
	_bsp_dcache_info.linesize = 32;
	_bsp_dcache_info.ways = 2;
	_bsp_icache_info.size = 4096;
	_bsp_icache_info.linesize = 32;
	_bsp_icache_info.ways = 2;
	break;
      case 1:
	_bsp_platform_info.cpu = "MB86832";
	_bsp_dcache_info.size = 8192;
	_bsp_dcache_info.linesize = 32;
	_bsp_dcache_info.ways = 2;
	_bsp_icache_info.size = 8192;
	_bsp_icache_info.linesize = 32;
	_bsp_icache_info.ways = 2;
	break;
      case 2:
	_bsp_platform_info.cpu = "MB86833";
	_bsp_dcache_info.size = 1024;
	_bsp_dcache_info.linesize = 16;
	_bsp_dcache_info.ways = 1;
	_bsp_icache_info.size = 1024;
	_bsp_icache_info.linesize = 16;
	_bsp_icache_info.ways = 1;
	break;
    }


    init_ms_timer();

    kill_hook();

    _bsp_kill_hook = kill_hook;

    sw1 = ~(*(unsigned char *)SW1_ADDR);
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
}

