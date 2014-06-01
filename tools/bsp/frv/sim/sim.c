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


static void
uart_irq_enable(void *chan)
{
}

static int
uart_irq_disable(void *chan)
{
    return 0;
}


static void
__outbyte(char *ch)
{
    asm("setlos	5,gr7\n"
	"ori    gr8,0,gr9\n"
	"setlos 1,gr8\n"
	"setlos 1,gr10\n"
	"tira	gr0,#0\n");
}

static void
uart_putchar(void *chan_data, char ch)
{
    __outbyte(&ch);
}


static void
uart_write(void *chan_data, const char *buf, int len)
{
    while (len-- > 0)
	uart_putchar(chan_data, *buf++);
}


static void
__inbyte(char *ch)
{
    asm("setlos	4,gr7\n"
	"ori    gr8,0,gr9\n"
	"setlos 0,gr8\n"
	"setlos 1,gr10\n"
	"tira	gr0,#0\n");
}

static int
uart_rcvchar(void *chan_data)
{
    char ch;
    __inbyte(&ch);
    return ch & 0xff;
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
	uart_irq_enable(chan_data);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	uart_irq_disable(chan_data);
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
    { (void *)0, (void *)0x00000000, 0, 0, BSP_MEM_RAM }
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
    _bsp_platform_info.cpu = "Generic FR-V";
    _bsp_platform_info.board = "Simulator";

    kill_hook();

    _bsp_kill_hook = kill_hook;
}


/*
 * Flush Icache for given range. If nbytes < 0, flush entire cache.
 */
void
__icache_flush(void *addr, int nbytes)
{
}


/*
 * Flush Dcache for given range. If nbytes < 0, flush entire cache.
 */
void
__dcache_flush(void *addr, int nbytes)
{
}
