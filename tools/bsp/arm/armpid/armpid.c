/*
 * armpid.c -- Support for ARM(R) Pid Eval Board
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include __BOARD_HEADER__
#include <stdlib.h>
#include "queue.h"
#include "bsp_if.h"

/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)RAM_VIRTUAL_BASE, (void *)RAM_VIRTUAL_BASE, 0, RAM_TOTAL_SIZE, BSP_MEM_RAM },
    { (void *)ROM_VIRTUAL_BASE, (void *)ROM_VIRTUAL_BASE, 0, ROM_TOTAL_SIZE, BSP_MEM_ROM },
    { (void *)0x00080000, (void *)0x00080000, 0, 0x00F80000, BSP_MEM_RAM },
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = sizeof(_bsp_memory_list)/sizeof(_bsp_memory_list[0]);

/*
 * Toggle LED for debugging purposes.
 */
void flash_led(int n, int which)
{
    int led;

    if ((which < 0) || (which > 3))
        return;

    /*
     * Make sure the Parallel Port is in Write mode
     */
    *ARMPID_PAR_CONTROL &= ~ARMPID_PAR_CONTROL_DATA_DIR;

    /*
     * Double n so we turn on and off for each count
     */
    n <<= 1;

    /*
     * Select the right LED
     */
    led = 0x10 << which;

    while (n--)
    {
        int i;

        *ARMPID_PAR_DATA ^= led;
        i = 0xffff; while (--i);
    }
}

/*
 * Enable/Disable separate debug functions
 */
#define DEBUG_UART_CODE      0
#define DEBUG_UART_FUNCTIONS 0
#define DEBUG_LOCAL_ECHO     0

static void	armpid_ictrl_init(const struct bsp_irq_controller *ic);
static int	armpid_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr);
static void	armpid_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr);

static bsp_vec_t *armpid_irq_vec_tbl[NUM_ARMPID_INTERRUPTS];

static const struct bsp_irq_controller armpid_irq_controller = {
    ARMPID_IRQ_MIN, ARMPID_IRQ_MAX,
    &armpid_irq_vec_tbl[0],
    armpid_ictrl_init,
    armpid_ictrl_disable,
    armpid_ictrl_enable
};


/*
 * Dispatch code for ARM(R) Pid interrupts. Called by exception dispatch code.
 */
static int
armpid_irq_dispatch(int ex_nr, ex_regs_t *regs)
{
    bsp_vec_t *vec = NULL;
    int done = 0;
    int irq = 0;

#if DEBUG_UART_CODE
    bsp_printf("armpid_irq_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
    BSP_ASSERT((ex_nr == BSP_CORE_EXC_IRQ) || (ex_nr == BSP_CORE_EXC_FIQ));

    bsp_printf("*ARMPID_IRQCONT_IRQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_IRQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_FIQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSOURCE = 0x%04lx\n", *ARMPID_IRQCONT_FIQSOURCE);
#endif /* DEBUG_UART_CODE */

    if (ex_nr == BSP_CORE_EXC_FIQ)
    {
        irq = ARMPID_IRQ_FIQ;
    } else {
        /*
         * Some IRQ occurred.
         * Figure out which one.
         */
        unsigned long int_status = *ARMPID_IRQCONT_IRQSTATUS;

        /*
         * We will prioritize them by the low-order bit that is
         * set in the status register
         */
        while (((int_status & 0x1) == 0x0) && (irq < NUM_ARMPID_INTERRUPTS))
        {
            irq++;
            int_status >>= 1;
        }
    }

#if DEBUG_UART_CODE
    bsp_printf("irq == %d\n", irq);
#endif /* DEBUG_UART_CODE */

    if (irq < NUM_ARMPID_INTERRUPTS)
        vec = armpid_irq_vec_tbl[irq];

    while (!done && vec && vec->handler != NULL) {
#if DEBUG_UART_CODE
        bsp_printf("vec->handler = 0x%08lx\n", vec->handler);
        bsp_printf("vec->next = 0x%08lx\n", vec->next);
#endif /* DEBUG_UART_CODE */
        done = vec->handler(ex_nr, regs);
        vec = vec->next;
    }

    return done;
}


/*
 *  Initialize ARM(R) Pid interrupt controller.
 */
static void
armpid_ictrl_init(const struct bsp_irq_controller *ic)
{
    static bsp_vec_t irq_vec, fiq_vec;
    int i;

#if DEBUG_UART_CODE
    bsp_printf("armpid_ictrl_init(0x%08lx)\n", ic);

    bsp_printf("*ARMPID_IRQCONT_IRQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_IRQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_FIQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSOURCE = 0x%04lx\n", *ARMPID_IRQCONT_FIQSOURCE);
#endif /* DEBUG_UART_CODE */

    /*
     * Clear all pending interrupts
     */
    *ARMPID_IRQCONT_IRQENABLECLEAR = ~0;
    *ARMPID_IRQCONT_FIQENABLECLEAR = ~0;

    /*
     * Set the FIQ Source to be the external pin
     */
    *ARMPID_IRQCONT_FIQSOURCE = ARMPID_IRQ_FIQ_EXTERNAL;

    for (i = ic->first; i < ic->last; i++)
    {
        ic->vec_list[i - ic->first] = NULL;
    }

    /*
     * Setup exception handlers for the exceptions
     * corresponding to IRQ's and FIQ's
     */
    irq_vec.handler = (void *)armpid_irq_dispatch;
    irq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_IRQ, BSP_VEC_CHAIN_FIRST, &irq_vec);

    fiq_vec.handler = (void *)armpid_irq_dispatch;
    fiq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_FIQ, BSP_VEC_CHAIN_FIRST, &fiq_vec);
}


/*
 *  Disable ARM(R) Pid interrupts.
 */
static int
armpid_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if DEBUG_UART_CODE
    BSP_ASSERT((irq_nr >= 0) && (irq_nr <= NUM_ARMPID_INTERRUPTS));
    bsp_printf("armpid_ictrl_disable(0x%08lx, %d)\n", ic, irq_nr);

    bsp_printf("*ARMPID_IRQCONT_IRQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_IRQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_FIQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSOURCE = 0x%04lx\n", *ARMPID_IRQCONT_FIQSOURCE);
#endif /* DEBUG_UART_CODE */

    *ARMPID_IRQCONT_IRQENABLECLEAR = ARMPID_IRQ_INTSRC_MASK(irq_nr);
    return 0;
}


/*
 *  Enable ARM(R) Pid interrupts.
 */
static void
armpid_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if DEBUG_UART_CODE
    bsp_printf("armpid_ictrl_enable(0x%08lx, %d)\n", ic, irq_nr);
    BSP_ASSERT((irq_nr >= 0) && (irq_nr <= NUM_ARMPID_INTERRUPTS));

    bsp_printf("*ARMPID_IRQCONT_IRQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_IRQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_IRQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_IRQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQRAWSTATUS = 0x%04lx\n", *ARMPID_IRQCONT_FIQRAWSTATUS);
    bsp_printf("*ARMPID_IRQCONT_FIQENABLE = 0x%04lx\n", *ARMPID_IRQCONT_FIQENABLE);
    bsp_printf("*ARMPID_IRQCONT_FIQSOURCE = 0x%04lx\n", *ARMPID_IRQCONT_FIQSOURCE);
#endif /* DEBUG_UART_CODE */

    *ARMPID_IRQCONT_IRQENABLESET = ARMPID_IRQ_INTSRC_MASK(irq_nr);
}

/*
 * Macros to make finding the registers easier
 */
#define UART_INDEX(base)                 (((unsigned)(base) == UART_BASE_0) ? 0 : 1)
#define UART_BASE_VALID(base)            (((unsigned)(base) == UART_BASE_0) || \
                                          ((unsigned)(base) == UART_BASE_1))
#define UART_RX_HOLDING(base)            REG_PTR((unsigned)(base) + ARMPID_SER_RX_HOLDING_o)
#define UART_TX_HOLDING(base)            REG_PTR((unsigned)(base) + ARMPID_SER_TX_HOLDING_o)
#define UART_INT_ENABLE(base)            REG_PTR((unsigned)(base) + ARMPID_SER_INT_ENABLE_o)
#define UART_INT_STATUS(base)            REG_PTR((unsigned)(base) + ARMPID_SER_INT_STATUS_o)
#define UART_FIFO_CONTROL(base)          REG_PTR((unsigned)(base) + ARMPID_SER_FIFO_CONTROL_o)
#define UART_LINE_CONTROL(base)          REG_PTR((unsigned)(base) + ARMPID_SER_LINE_CONTROL_o)
#define UART_MODEM_CONTROL(base)         REG_PTR((unsigned)(base) + ARMPID_SER_MODEM_CONTROL_o)
#define UART_LINE_STATUS(base)           REG_PTR((unsigned)(base) + ARMPID_SER_LINE_STATUS_o)
#define UART_MODEM_STATUS(base)          REG_PTR((unsigned)(base) + ARMPID_SER_MODEM_STATUS_o)
#define UART_SCRATCHPAD(base)            REG_PTR((unsigned)(base) + ARMPID_SER_SCRATCHPAD_o)
#define UART_DIVISOR_LSB(base)           REG_PTR((unsigned)(base) + ARMPID_SER_DIVISOR_LSB_o)
#define UART_DIVISOR_MSB(base)           REG_PTR((unsigned)(base) + ARMPID_SER_DIVISOR_MSB_o)
#define UART_INT_NUM(base)               (((unsigned)(base) == UART_BASE_0) ? \
                                          ARMPID_IRQ_SERIAL_PORT_A :          \
                                          ARMPID_IRQ_SERIAL_PORT_B)

/*
 * Function prototypes
 */
static void uart_write   (void*, const char*, int);
static int  uart_read    (void*, char*, int);
static void uart_putchar (void*, char);
static int  uart_getchar (void*);
static int  uart_control (void*, int, ...);
#if DEBUG_UART_FUNCTIONS
void uart_putnibble(void*, unsigned char);
void uart_putbyte(void*, unsigned char);
void uart_putword(void*, unsigned short);
void uart_putlong(void*, unsigned long);
void uart_putstring(void *, char *);
void uart_putcharacter(void *, char);
#endif /* DEBUG_UART_FUNCTIONS */

/*
 * Setup the bsp_comm_channel data structure
 */
#define BSP_NUM_COMMS        2
struct bsp_comm_channel _bsp_comm_list[BSP_NUM_COMMS] = 
{
    {
	{ "Serial Port A", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE_0, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    },
    {
	{ "Serial Port B", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE_1, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
};
int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);

/*
 * Baud rate selection stuff
 */
static int uart_baud[BSP_NUM_COMMS] = {-1,  -1};
struct _baud {
    int baud;
    unsigned short clock_select_value;
};

const static struct _baud bauds[] = {
    {    50, 0x0900},
    {   110, 0x0417},
    {   150, 0x0300},
    {   300, 0x0180},
    {   600, 0x00c0},
    {  1200, 0x0060},
    {  2400, 0x0030},
    {  4800, 0x0018},
    {  7200, 0x0010},
    {  9600, 0x000c},
    { 19200, 0x0006},
    { 38400, 0x0003},
    { 57600, 0x0002},
    {115200, 0x0001},
};

/*
 * UART queuing stuff
 */
#define UART_BUFSIZE 32
static bsp_queue_t uart_queue[BSP_NUM_COMMS];
static char uart_buffer[BSP_NUM_COMMS][UART_BUFSIZE];

/*
 * The currently selected debug port
 */
static volatile unsigned char *uart_dbg_base = NULL;

static void
uart_irq_enable(void *base)
{
    if (UART_BASE_VALID(base))
    {
        *UART_INT_ENABLE(base) |= (ARMPID_SER_RX_HOLDING_INT_ENABLE | \
				   ARMPID_SER_RX_LINE_STAT_INT_ENABLE);
    }
}


static int
uart_irq_disable(void *base)
{
    int was_enabled = 0;

    if (UART_BASE_VALID(base))
    {
	was_enabled = *UART_INT_ENABLE(base) &
	    (ARMPID_SER_RX_HOLDING_INT_ENABLE | \
	     ARMPID_SER_RX_LINE_STAT_INT_ENABLE);
        *UART_INT_ENABLE(base) = 0;
    }

    return was_enabled != 0;
}

static void uart_putchar(void *base, char c)
{
    if (UART_BASE_VALID(base))
    {
	int was_enabled = uart_irq_disable(base);

        while ((*UART_LINE_STATUS(base) & ARMPID_SER_TX_HOLDING_EMPTY) == 0)
            ;
        *UART_TX_HOLDING(base) = c;

	if (was_enabled)
	    uart_irq_enable(base);
    }
}


static void
uart_write(void *base, const char *buf, int len)
{
    if (UART_BASE_VALID(base))
    {
        while (len-- > 0)
            uart_putchar(base, *buf++);
    }
}

static int
uart_rcvchar(void *base)
{
    if (UART_BASE_VALID(base))
    {
        while ((*UART_LINE_STATUS(base) & ARMPID_SER_RX_DATA_READY) == 0)
            ;
        return((*UART_RX_HOLDING(base)) & 0xFFFF);
    }

    return(-1);
}

static int
uart_getchar(void *base)
{
    int c, was_enabled;

    if (UART_BASE_VALID(base))
    {
        was_enabled = uart_irq_disable(base);
        if ((c = _bsp_dequeue(&uart_queue[UART_INDEX(base)])) < 0)
            c = uart_rcvchar (base);
	if (was_enabled)
	    uart_irq_enable (base);

#if DEBUG_LOCAL_ECHO
        uart_putchar(base, c);
#endif
        return c;
    }

    return (-1);
}

static int
uart_read(void *base, char *buf, int len)
{
    int ch;

    if (UART_BASE_VALID(base))
    {
        if (len <= 0)
            return 0;

        ch = uart_getchar(base);
        *buf = ch;
        return 1;
    }

    return 0;
}

static int
uart_irq_handler(int irq_nr, void *regs)
{
    unsigned char ch;
    int return_code = 1;
    unsigned long intstatus = *UART_INT_STATUS(uart_dbg_base) & 0xF;

#if DEBUG_UART_CODE
    bsp_printf("In uart_irq_handler(%d, 0x%08lx)\n", irq_nr, regs);
    _bsp_dump_regs(regs);
#endif /* DEBUG_UART_CODE */

    if (intstatus == ARMPID_SER_INT_SRC_LSR)
    {
	ch = *UART_LINE_STATUS(uart_dbg_base);
    }
    else if ((intstatus == ARMPID_SER_INT_SRC_RXRDY) ||
	     (intstatus == ARMPID_SER_INT_SRC_RX_TIMEOUT))
    {
        ch = uart_rcvchar ((void *) uart_dbg_base);

        /*
         * If we read a Ctrl-C then we need to let
         * GDB handle it.  Return 0 to have the GDB
         * exception processor run.
         */
        if (ch == '\003')
        {
	  return_code = 0;
        }
	else
	{
	  _bsp_enqueue(&uart_queue[UART_INDEX(uart_dbg_base)], ch);
#if DEBUG_LOCAL_ECHO
            uart_putchar(uart_dbg_base, ch);
#endif
        }
    }
  
    return return_code;
}

static int
uart_setbaud(void *base, int bps)
{
    int   i;
    const struct _baud *bp;
  
    if ((UART_BASE_VALID(base)) && (uart_baud[UART_INDEX(base)] != bps))
    {
        for (i = 0, bp = bauds; i < (sizeof(bauds)/sizeof(bauds[0])); i++, bp++)
        {
            if (bp->baud == bps)
            {
                /*
                 * Setup the UART Baud Rate Divisor
                 */
                *UART_DIVISOR_MSB(base) = bp->clock_select_value >> 8;
                *UART_DIVISOR_LSB(base) = bp->clock_select_value & 0xFF;

                /*
                 * Store the state
                 */
                uart_baud[UART_INDEX(base)] = bps;
                return 0;
            }
        }
    }

    return -1;
}

static int
uart_control(void *base, int func, ...)
{
    int                    i, retval = 0;
    va_list                ap;
    static bsp_vec_t       uart_vec;

    if (UART_BASE_VALID(base))
    {
        va_start (ap, func);

        switch (func) {
        case COMMCTL_SETBAUD:
            i = va_arg(ap, int);
            retval = uart_setbaud((void *)base, i);
            break;

        case COMMCTL_GETBAUD:
            retval = uart_baud[UART_INDEX(base)];
            break;

        case COMMCTL_INSTALL_DBG_ISR:
            if (uart_dbg_base != base) {
                if (uart_dbg_base == NULL) {
                    uart_vec.handler = uart_irq_handler;
                    uart_vec.next = NULL;
                    bsp_install_vec(BSP_VEC_INTERRUPT, UART_INT_NUM(base),
                                    BSP_VEC_REPLACE, &uart_vec);
                    bsp_enable_irq(UART_INT_NUM(base));
                    uart_irq_enable(base);
                }
                uart_dbg_base = base;
            }
            break;
	
        case COMMCTL_REMOVE_DBG_ISR:
            uart_irq_disable(base);
            bsp_disable_irq(UART_INT_NUM(base));
            bsp_remove_vec(BSP_VEC_INTERRUPT, UART_INT_NUM(base), &uart_vec);
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
    } else {
        retval = -1;
    }

    return retval;
}


#if DEBUG_UART_FUNCTIONS
#else
static
#endif /* DEBUG_UART_FUNCTIONS */
void
init_uart(void *base, int bps)
{
    unsigned long dummy;

    if (UART_BASE_VALID(base))
    {
        /*
         * Clear pending irq
         */
        dummy = *UART_INT_STATUS(base);
        
        /*
         * Clear modem status
         */
        dummy = *UART_MODEM_STATUS(base);

        /*
         * Enable the interrupt out lines from the uart
         */
        *UART_MODEM_CONTROL(base) = ARMPID_SER_INT_OUTPUT_ENABLE | 
                                    ARMPID_SER_DTR_FORCE_LOW     |
                                    ARMPID_SER_RTS_FORCE_LOW;

        /*
         * Reset and enable the FIFO's
         */
        *UART_FIFO_CONTROL(base) = ARMPID_SER_FIFO_ENABLE     |
                                   ARMPID_SER_RX_FIFO_RESET   |
                                   ARMPID_SER_TX_FIFO_RESET   |
                                   ARMPID_SER_RX_TRIGGER_01;

        /*
         * Enable Baud Divisor Latch
         */
        *UART_LINE_CONTROL(base) = ARMPID_SER_DIVISOR_LATCH_ENABLE;

        /*
         * Set the baud rate
         */
        uart_setbaud((void *)base, bps);

        /*
         * Disable all interrupts
         */
        *UART_INT_ENABLE(base) = 0;

        /*
         * Set the data and stop bits
         */
        *UART_LINE_CONTROL(base) = ARMPID_SER_DATABITS_8_STOPBITS_1;
    }
}

/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    _bsp_queue_init(&uart_queue[0], uart_buffer[0], sizeof(uart_buffer[0]));
    _bsp_queue_init(&uart_queue[1], uart_buffer[1], sizeof(uart_buffer[1]));

    init_uart((void *)UART_BASE_0, 38400);
    init_uart((void *)UART_BASE_1, 38400);
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
#if DEBUG_UART_CODE
    bsp_printf("_bsp_install_board_irq_controllers()\n");
#endif /* DEBUG_UART_CODE */

    _bsp_install_irq_controller(&armpid_irq_controller);
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
    _bsp_platform_info.cpu = "ARM7TDMI";
    _bsp_platform_info.board = "ARM PID";
}

#if DEBUG_UART_FUNCTIONS
/*
 * Print out a single Hex digit
 */
void uart_putnibble(void *base, unsigned char printval)
{
    if (printval <= 9)
        uart_putchar(base, printval + '0');
    else if ((printval >= 0xA) && (printval <= 0xF))
        uart_putchar(base, printval - 0xA + 'A');
    else
        uart_putchar(base, '.');
}

/*
 * Print out a byte value
 */
void uart_putbyte(void *base, unsigned char printval)
{
    uart_putnibble(base, (unsigned char)((printval >> 4)  & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 0)  & 0x0000000F));
}

/*
 * Print out a word value
 */
void uart_putword(void *base, unsigned short printval)
{
    uart_putnibble(base, (unsigned char)((printval >> 12) & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 8)  & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 4)  & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 0)  & 0x0000000F));
}

/*
 * Print out a long value
 */
void uart_putlong(void *base, unsigned long printval)
{
    uart_putnibble(base, (unsigned char)((printval >> 28) & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 24) & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 20) & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 16) & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 12) & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 8)  & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 4)  & 0x0000000F));
    uart_putnibble(base, (unsigned char)((printval >> 0)  & 0x0000000F));
}


/*
 * Print out a NULL terminated string
 */
void uart_putstring(void *base, char *string)
{
    if (string != NULL)
    {
        while (*string != '\0')
        {
            uart_putchar(base, *string);
            string++;
        }
    }
}

/*
 * Print out a single character
 */
void uart_putcharacter(void *base, char c)
{
    uart_putchar(base, c);
}
#endif /* DEBUG_UART_FUNCTIONS */
