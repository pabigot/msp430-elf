/*
 * sim.c -- Support for simulator based stub.
 *
 * Copyright (c) 1999, 2000 Cygnus Support
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
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "queue.h"


typedef struct sio_port {
    volatile unsigned lsr;
    volatile unsigned char txd;
    volatile unsigned char txpad[3];
    volatile unsigned char rxd;
    volatile unsigned char rxpad[3];
} sio_t;

#define TXRDY 1
#define RXRDY 2

#define SIO_CHAN ((void*)0xffffff00)

static void
uart_irq_enable(void *chan)
{
}

static int
uart_irq_disable(void *chan)
{
    return 0;
}

#if !defined(__DUMMY_SIM_UART__)
static void
__outbyte(char ch)
{
    asm volatile (".word 0x00000605\n");
}
#endif

static void
uart_putchar(void *chan_data, char ch)
{
#if defined(__DUMMY_SIM_UART__)
    sio_t *p = (sio_t *)chan_data;

    while (!(p->lsr & TXRDY))
	;

    p->txd = ch;
#else
    __outbyte(ch);
#endif
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
#if defined(__DUMMY_SIM_UART__)
    sio_t *p = (sio_t *)chan_data;

    while (!(p->lsr & RXRDY))
	;

    return p->rxd;
#else
    /* use sim idt monitor support */
    asm volatile (".word 0x00000585\n");
#endif
}


static int
uart_getchar(void *chan_data)
{
    int ch, was_disabled;

    was_disabled = uart_irq_disable(chan_data);

    ch = uart_rcvchar(chan_data);

    if (was_disabled)
	uart_irq_enable(chan_data);
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
    return 0;
}

static const int _bauds[] = {
    300, 600, 1200, 2400, 4800, 9600,
    19200, 38400, 0
};

static int uart_baud;

static int
uart_setbaud(void *chan, int bps)
{
    int i;

    for (i = 0; _bauds[i]; i++) {
	if (_bauds[i] == bps) {
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
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(chan_data, i);
	break;

      case COMMCTL_GETBAUD:
	retval = uart_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = uart_irq_handler;
	uart_vec.next = NULL;
#if 0
	bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SIO,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(BSP_IRQ_SIO);
#endif
	uart_irq_enable(chan_data);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	uart_irq_disable(chan_data);
#if 0
	bsp_disable_irq(BSP_IRQ_SIO);
	bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SIO, &uart_vec);
#endif
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = uart_irq_disable(chan_data);
	break;

      case COMMCTL_IRQ_ENABLE:
	uart_irq_enable(chan_data);
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
	{ SIO_CHAN, uart_write, uart_read,
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
    uart_setbaud(0, 38400);
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
    { (void *)0, (void *)0x80000000, 0, 0, BSP_MEM_RAM }
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = 1;


extern void (*_bsp_kill_hook)(void);
extern void _bsp_install_vsr(void);

static void
kill_hook(void)
{
    _bsp_install_vsr();
    bsp_debug_irq_enable();
#ifdef NUM_TLB_ENTRIES
    __tlb_init();
#endif
}


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
    _bsp_platform_info.cpu = "Generic MIPS";
    _bsp_platform_info.board = "Simulator";

    kill_hook();

    _bsp_kill_hook = kill_hook;
}


#define ICACHE_SIZE    (16*1024)
#define ICACHE_LSIZE   (32)
#define ICACHE_WAYS    4
#define ICACHE_INDICES (ICACHE_SIZE / (ICACHE_LSIZE*ICACHE_WAYS))

/*
 * Flush Icache for given range. If nbytes < 0, flush entire cache.
 */
void
__icache_flush(void *addr, int nbytes)
{
    int   i;

    if (nbytes == 0)
	return;

    /* flush entire cache */
#ifdef __mips64
    addr = (void *)0xffffffff80000000L;
#else
    addr = (void *)0x80000000L;
#endif
    for (i = 0; i < ICACHE_INDICES; i++, addr += ICACHE_LSIZE) {
	asm volatile ("\t.set push\n"
		      "\t.set mips3\n"
		      "\tcache	%1,0(%0)\n"
		      "\tori	%0,%0,0x1000\n"
		      "\tcache	%1,1(%0)\n"
		      "\txori	%0,%0,0x1000\n"
		      "\tori	%0,%0,0x2000\n"
		      "\tcache	%1,2(%0)\n"
		      "\tori	%0,%0,0x1000\n"
		      "\tcache	%1,3(%0)\n"
		      "\txori	%0,%0,0x3000\n"
		      "\t.set pop\n"
		      : /* no outputs */
		      : "r" (addr), "i" (INDEX_INVALIDATE_I) );
    }
}


#define DCACHE_SIZE    (16*1024)
#define DCACHE_LSIZE   (32)
#define DCACHE_WAYS    4
#define DCACHE_INDICES (DCACHE_SIZE / (DCACHE_LSIZE*DCACHE_WAYS))

/*
 * Flush Dcache for given range. If nbytes < 0, flush entire cache.
 */
void
__dcache_flush(void *addr, int nbytes)
{
    int i;

    if (nbytes == 0)
	return;

#ifdef __mips64
    addr = (void *)0xffffffff80000000L;
#else
    addr = (void *)0x80000000L;
#endif
    for (i = 0; i < DCACHE_INDICES; i++, addr += DCACHE_LSIZE) {
	asm volatile ("\t.set push\n"
		      "\t.set mips3\n"
		      "\tcache	%1,0(%0)\n"
		      "\tori	%0,%0,0x1000\n"
		      "\tcache	%1,1(%0)\n"
		      "\txori	%0,%0,0x1000\n"
		      "\tori	%0,%0,0x2000\n"
		      "\tcache	%1,2(%0)\n"
		      "\tori	%0,%0,0x1000\n"
		      "\tcache	%1,3(%0)\n"
		      "\txori	%0,%0,0x3000\n"
		      "\t.set pop\n"
		      : /* no outputs */
		      : "r" (addr), "i" (INDEX_WRITEBACK_INVALIDATE_D) );
    }
}
