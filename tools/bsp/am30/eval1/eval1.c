/*
 * eval1.c -- MN103002A eval board initialization.
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
#include <stdlib.h>
#include "queue.h"
#include "bsp_if.h"

/*
 * Timer 2(3) and Serial Port 2 timer values used to program various baud
 * rates for Serial Port 2.  These are used to divide down the IOCLK
 * frequency to the desired baud rate.
 *
 * To calculate the values, use this equation:
 *
 *    15000000 / baud = (TM2_Bnnnn + 1) * (SC2_Bnnnn + 1)
 *
 * Where 15000000 is the IOCLK rate (15MHz), TM2_Bnnnn is the value written
 * to timer 2 (TM2BR), and SC2_Bnnnn is the value written to Serial Port
 * 2's timer register (SC2TIM).  The SC2_Bnnnn value cannot be greater
 * than 127.  The values written to the timer registers are one
 * less than the actual calculated ratios, hence the + 1 in the
 * equation above.
 *
 * Consider 9600 baud as an example.  15M/9600 is 1562, which can
 * be factored as 22 * 71.  So the value written to Timer 2 is 21,
 * and the value written to Serial 2 Timer is 70.
 *
 * Note that there can be several ways to factor out the division ratio.
 * I have picked values that come closer than the ones suggested
 * by Matsushita in Table 11-5-4 of the MN10300 manual.
 */

static int uart1_baud, uart2_baud;

static int
uart1_setbaud(int bps)
{
    switch (bps) {
      case 9600:
	am30_tm1br = 194;
	break;
      case 19200:
	am30_tm1br = 97;
	break;
      case 38400:
	am30_tm1br = 48;
	break;
      default:
	return -1;
    }
    uart1_baud = bps;
    return 0;
}


static int
uart2_setbaud(int bps)
{
    switch (bps) {
      case 9600:
	am30_tm2br = 21;
	am30_sio2->tim = 70;
	break;
      case 19200:
	am30_tm2br = 10;
	am30_sio2->tim = 70;
	break;
      case 38400:
	am30_tm2br = 16;
	am30_sio2->tim = 22;
	break;
      default:
	return -1;
    }
    uart2_baud = bps;
    return 0;
}


/*
 * Initialize serial port with the specified baud rate.
 */
static void
init_serial1(int baudrate)
{
    uart1_setbaud(baudrate);

    am30_tm1md = TMR8_MD_ENABLE | TMR8_MD_IOCLK;

    am30_sio1->icr  = 0;
    am30_sio1->ctrl = SIO_RX_ENABLE | SIO_TX_ENABLE | SIO_CLK_TMR1_8 |
	         SIO_DATA_8 | SIO_PAR_NONE | SIO_STOP1;

    /* enable uart output on port3 I/O pin. */
    am30_p3md = 1;
}


/*
 * Initialize serial port with the specified baud rate.
 */
static void
init_serial2(int baudrate)
{
    uart2_setbaud(baudrate);

    am30_tm2md = TMR8_MD_ENABLE | TMR8_MD_IOCLK;

    am30_sio2->icr  = 0;
    am30_sio2->ctrl = SIO_RX_ENABLE | SIO_TX_ENABLE | SIO2_CLK_TMR2 |
	         SIO_DATA_8 | SIO_PAR_NONE | SIO_STOP1;
}


/*
 *  Send a byte out the GDB UART.
 */
static void
uart1_putchar(void *unused, char ch)
{
    while (am30_sio1->sts & SIO_STS_TXFULL)
	;

    am30_sio1->txd = ch & 0xff;
}

static void
uart2_putchar(void *unused, char ch)
{
    while (am30_sio2->sts & SIO_STS_TXFULL)
	;

    am30_sio2->txd = ch & 0xff;
}


static void
uart_write(void *chan, const char *buf, int len)
{
    if (chan) {
	while (len-- > 0)
	    uart2_putchar(chan, *buf++);
    } else {
	while (len-- > 0)
	    uart1_putchar(chan, *buf++);
    }
}


/*
 *  Wait for an incoming byte from the GDB UART.
 */
static int
uart1_getchar(void *unused)
{
    while (!(am30_sio1->sts & SIO_STS_RXFULL))
	;

    /*
     *  It appears neccessary to clear the interrupt bit
     *  even though we're polling and interrupts are
     *  disabled.
     */
    am30_icr[18].icr &= ~0x10;
  
    return am30_sio1->rxd & 0xff;
}


static int
uart2_getchar(void *unused)
{
    while (!(am30_sio2->sts & SIO_STS_RXFULL))
	;

    /*
     *  It appears neccessary to clear the interrupt bit
     *  even though we're polling and interrupts are
     *  disabled.
     */
    am30_icr[20].icr &= ~0x10;
  
    return am30_sio2->rxd & 0xff;
}

static int
uart_read(void *chan, char *buf, int len)
{
    int ch;

    if (len <= 0)
	return 0;

    if (chan)
	ch = uart2_getchar(chan);
    else
	ch = uart1_getchar(chan);
    *buf = ch;
    return 1;
}


/*
 *  Handle uart RX interrupts.
 */
static int
uart1_irq_handler(int irq_nr, void *regs_ptr)
{
    am30_icr[18].icr &= ~0x10;

    /*
     * If we got a Control-C character, pretend we didn't handle
     * the interrupt so that debug stub is invoked by low-level
     * interrupt code.
     */
    if (am30_sio1->sts & SIO_STS_RXFULL)
	if (am30_sio1->rxd == '\003')
	    return 0;

    return 1;
}

/*
 *  Handle uart RX interrupts.
 */
static int
uart2_irq_handler(int irq_nr, void *regs_ptr)
{
    am30_icr[20].icr &= ~0x10;

    /*
     * If we got a Control-C character, pretend we didn't handle
     * the interrupt so that debug stub is invoked by low-level
     * interrupt code.
     */
    if (am30_sio2->sts & SIO_STS_RXFULL)
	if (am30_sio2->rxd == '\003')
	    return 0;

    return 1;
}


static int
uart_control(void *chan, int func, ...)
{
    int          i, retval = 0;
    va_list      ap;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	if (chan)
	    retval = uart2_setbaud(i);
	else
	    retval = uart1_setbaud(i);
	break;

      case COMMCTL_GETBAUD:
	retval = (chan) ? uart2_baud : uart1_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	if (chan) {
	    uart_vec.handler = uart2_irq_handler;
	    i = BSP_IRQ_SIO2_RX;
	} else {
	    uart_vec.handler = uart1_irq_handler;
	    i = BSP_IRQ_SIO1_RX;
	}

	uart_vec.next = NULL;
	bsp_install_vec(BSP_VEC_INTERRUPT, i,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(i);
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	if (chan)
	    i = BSP_IRQ_SIO2_RX;
	else
	    i = BSP_IRQ_SIO1_RX;
	bsp_disable_irq(i);
	bsp_remove_vec(BSP_VEC_INTERRUPT, i, &uart_vec);
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
	{ (void*)0, uart_write, uart_read,
	  uart1_putchar, uart1_getchar, uart_control }
    },
    {
	{ "uart2", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)1, uart_write, uart_read,
	  uart2_putchar, uart2_getchar, uart_control }
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
    init_serial1(38400);
    init_serial2(38400);
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
    { (void *)0x48000000, (void *)0x48000000, 0, 0, BSP_MEM_RAM }
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
    _bsp_platform_info.cpu = "AM32";
    _bsp_platform_info.board = "Matsushita Eval1";

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

}

