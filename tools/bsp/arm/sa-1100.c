/*
 * sa-1100.c -- Intel(R) SA-1100 specific support for Cygnus BSP
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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
 * Intel is a Registered Trademark of Intel Corporation.
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include <stdlib.h>
#include <bsp/sa-1100.h>
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "queue.h"

/*
 * Enable/Disable separate debug functions
 */
#define DEBUG_UART_CODE      0
#define DEBUG_UART_FUNCTIONS 0
#define DEBUG_LOCAL_ECHO     0

static void	sa1100_ictrl_init(const struct bsp_irq_controller *ic);
static int	sa1100_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr);
static void	sa1100_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr);

static bsp_vec_t *sa1100_irq_vec_tbl[NUM_SA1100_INTERRUPTS];

static const struct bsp_irq_controller sa1100_irq_controller = {
    SA1100_IRQ_MIN, SA1100_IRQ_MAX,
    &sa1100_irq_vec_tbl[0],
    sa1100_ictrl_init,
    sa1100_ictrl_disable,
    sa1100_ictrl_enable
};


/*
 * Dispatch code for sa1100 interrupts. Called by exception dispatch code.
 */
static int
sa1100_irq_dispatch(int ex_nr, ex_regs_t *regs)
{
    bsp_vec_t *vec = NULL;
    int done = 0;
    int irq = 0;

#if DEBUG_UART_CODE
    bsp_printf("sa1100_irq_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
    BSP_ASSERT((ex_nr == BSP_CORE_EXC_IRQ) || (ex_nr == BSP_CORE_EXC_FIQ));
#endif /* DEBUG_UART_CODE */

    if (ex_nr == BSP_CORE_EXC_FIQ)
    {
        /*
         * Some FIQ occurred.
         * Figure out which one.
         */
        unsigned long int_status = *SA1100_ICFP;

        /*
         * We will prioritize them by the low-order bit that is
         * set in the status register
         */
        while (((int_status & 0x1) == 0x0) && (irq < NUM_SA1100_INTERRUPTS))
        {
            irq++;
            int_status >>= 1;
        }
    } else {
        /*
         * Some IRQ occurred.
         * Figure out which one.
         */
        unsigned long int_status = *SA1100_ICIP;

        /*
         * We will prioritize them by the low-order bit that is
         * set in the status register
         */
        while (((int_status & 0x1) == 0x0) && (irq < NUM_SA1100_INTERRUPTS))
        {
            irq++;
            int_status >>= 1;
        }
    }

#if DEBUG_UART_CODE
    bsp_printf("irq == %d\n", irq);
#endif /* DEBUG_UART_CODE */

    if (irq < NUM_SA1100_INTERRUPTS)
        vec = sa1100_irq_vec_tbl[irq];

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
 *  Initialize SA1100 interrupt controller.
 */
static void
sa1100_ictrl_init(const struct bsp_irq_controller *ic)
{
    static bsp_vec_t irq_vec, fiq_vec;
    int i;

#if DEBUG_UART_CODE
    bsp_printf("sa1100_ictrl_init(0x%08lx)\n", ic);

    bsp_printf("*SA1100_ICPR = 0x%08lx\n", *SA1100_ICPR);
    bsp_printf("*SA1100_ICIP = 0x%08lx\n", *SA1100_ICIP);
    bsp_printf("*SA1100_ICFP = 0x%08lx\n", *SA1100_ICFP);
    bsp_printf("*SA1100_ICMR = 0x%08lx\n", *SA1100_ICMR);
    bsp_printf("*SA1100_ICLR = 0x%08lx\n", *SA1100_ICLR);
    bsp_printf("*SA1100_ICCR = 0x%08lx\n", *SA1100_ICCR);
#endif /* DEBUG_UART_CODE */

    /*
     * Mask all interrupts
     */
    *SA1100_ICMR = 0;

    /*
     * All interrupts to IRQ rather than FIQ
     */
    *SA1100_ICLR = 0;  

    for (i = ic->first; i < ic->last; i++)
    {
        ic->vec_list[i - ic->first] = NULL;
    }

    /*
     * Setup exception handlers for the exceptions
     * corresponding to IRQ's and FIQ's
     */
    irq_vec.handler = (void *)sa1100_irq_dispatch;
    irq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_IRQ, BSP_VEC_CHAIN_FIRST, &irq_vec);

    fiq_vec.handler = (void *)sa1100_irq_dispatch;
    fiq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_FIQ, BSP_VEC_CHAIN_FIRST, &fiq_vec);
}


/*
 *  Disable SA1100 interrupts.
 */
static int
sa1100_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if DEBUG_UART_CODE
    BSP_ASSERT((irq_nr >= 0) && (irq_nr <= NUM_SA1100_INTERRUPTS));
    bsp_printf("sa1100_ictrl_disable(0x%08lx, %d)\n", ic, irq_nr);

    bsp_printf("*SA1100_ICIP = 0x%08lx\n", *SA1100_ICIP);
    bsp_printf("*SA1100_ICFP = 0x%08lx\n", *SA1100_ICFP);
    bsp_printf("*SA1100_ICMR = 0x%08lx\n", *SA1100_ICMR);
    bsp_printf("*SA1100_ICLR = 0x%08lx\n", *SA1100_ICLR);
    bsp_printf("*SA1100_ICCR = 0x%08lx\n", *SA1100_ICCR);
#endif /* DEBUG_UART_CODE */

    *SA1100_ICMR &= ~SA1100_IRQ_INTSRC_MASK(irq_nr);
    return 0;
}


/*
 *  Enable SA1100 interrupts.
 */
static void
sa1100_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if DEBUG_UART_CODE
    bsp_printf("sa1100_ictrl_enable(0x%08lx, %d)\n", ic, irq_nr);
    BSP_ASSERT((irq_nr >= 0) && (irq_nr <= NUM_SA1100_INTERRUPTS));

    bsp_printf("*SA1100_ICIP = 0x%08lx\n", *SA1100_ICIP);
    bsp_printf("*SA1100_ICFP = 0x%08lx\n", *SA1100_ICFP);
    bsp_printf("*SA1100_ICMR = 0x%08lx\n", *SA1100_ICMR);
    bsp_printf("*SA1100_ICLR = 0x%08lx\n", *SA1100_ICLR);
    bsp_printf("*SA1100_ICCR = 0x%08lx\n", *SA1100_ICCR);
#endif /* DEBUG_UART_CODE */

    *SA1100_ICMR |= SA1100_IRQ_INTSRC_MASK(irq_nr);
}

/*
 * SA1100 specific routine to 'install' interrupt controllers.
 */
void
_bsp_install_sa1100_irq_controllers(void)
{
#if DEBUG_UART_CODE
    bsp_printf("_bsp_install_sa1100_irq_controllers()\n");
    BSP_ASSERT(sa1100_irq_controller.first == SA1100_IRQ_MIN);
    BSP_ASSERT(sa1100_irq_controller.last  == SA1100_IRQ_MAX);
#endif /* DEBUG_UART_CODE */
    _bsp_install_irq_controller(&sa1100_irq_controller);
}

/*
 * Macros to make finding the registers easier
 */
#define UART_INDEX(base)                 (((unsigned long)(base) == (unsigned long)(UART_BASE_0)) \
                                           ? 0 : 1)
#define UART_BASE_VALID(base)            (((unsigned long)(base) == (unsigned long)(UART_BASE_0)) \
                                          ||                                                      \
                                          ((unsigned long)(base) == (unsigned long)(UART_BASE_1)))
#define UART_CONTROL_0(base)             REG32_PTR((unsigned long)(base) + SA1100_UART_CONTROL_0_o)
#define UART_CONTROL_1(base)             REG32_PTR((unsigned long)(base) + SA1100_UART_CONTROL_1_o)
#define UART_CONTROL_2(base)             REG32_PTR((unsigned long)(base) + SA1100_UART_CONTROL_2_o)
#define UART_CONTROL_3(base)             REG32_PTR((unsigned long)(base) + SA1100_UART_CONTROL_3_o)
#define UART_DATA(base)                  REG32_PTR((unsigned long)(base) + SA1100_UART_DATA_o)
#define UART_STATUS_0(base)              REG32_PTR((unsigned long)(base) + SA1100_UART_STATUS_0_o)
#define UART_STATUS_1(base)              REG32_PTR((unsigned long)(base) + SA1100_UART_STATUS_1_o)
#define UART_INT_NUM(base)               (((unsigned long)(base) == (unsigned long)(UART_BASE_0)) \
                                          ?                                                       \
                                          SA1100_IRQ_UART3_SERVICE_REQUEST :                      \
                                          SA1100_IRQ_UART1_SERVICE_REQUEST)
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
#ifdef __BOARD_SA1110AS__
#define BSP_NUM_COMMS        1
#else
#define BSP_NUM_COMMS        2
#endif
struct bsp_comm_channel _bsp_comm_list[BSP_NUM_COMMS] = 
{
#ifdef __BOARD_SA1110AS__
    {
	{ "Serial Port A", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE_1, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
#else
    {
	{ "Serial Port A", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE_0, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
    ,{
	{ "Serial Port B", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE_1, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
#endif
};
int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);

/*
 * Baud rate selection stuff
 */
static int uart_baud[BSP_NUM_COMMS] = {-1};
struct _baud {
    int baud;
    unsigned short divisor_high, divisor_low;
};

const static struct _baud bauds[] = {
    /* { 50, ,},  Too Small */            /* 4607  = 0x11FF */
    {    75, 0xB, 0xFF},                  /* 3071  = 0x0BFF */
    {   110, 0x8, 0x2E},                  /* 2094  = 0x082E */
    {   134, 0x6, 0xB0},                  /* 1712  = 0x06B0 */
    {   150, 0x5, 0xFF},                  /* 1535  = 0x05FF */
    {   300, 0x2, 0xFF},                  /* 767   = 0x02FF */
    {   600, 0x1, 0x7F},                  /* 383   = 0x017F */
    {  1200, 0x0, 0xBF},                  /* 191   = 0x00BF */
    {  1800, 0x0, 0x7F},                  /* 127   = 0x007F */
    {  2000, 0x0, 0x72},                  /* 114   = 0x0072 */
    {  2400, 0x0, 0x5F},                  /* 95    = 0x005F */
    {  3600, 0x0, 0x3F},                  /* 63    = 0x003F */
    {  4800, 0x0, 0x2F},                  /* 47    = 0x002F */
    {  7200, 0x0, 0x1F},                  /* 31    = 0x001F */
    {  9600, 0x0, 0x17},                  /* 23    = 0x0017 */
    { 19200, 0x0, 0x0B},                  /* 11    = 0x000B */
    { 38400, 0x0, 0x05},                  /* 5     = 0x0005 */
    { 56000, 0x0, 0x03},                  /* 3     = 0x0003 */
    { 57600, 0x0, 0x03},                  /* 3     = 0x0003 */
    {115200, 0x0, 0x01},                  /* 1     = 0x0001 */
    {128000, 0x0, 0x01},                  /* 1     = 0x0001 */
};

/*
 * UART queuing stuff
 */
#define UART_BUFSIZE 256
static bsp_queue_t uart_queue[2];
static char uart_buffer[2][UART_BUFSIZE];

/*
 * The currently selected debug port
 */
static volatile unsigned char *uart_dbg_base = NULL;

static void
uart_irq_enable(void *base)
{
    BSP_ASSERT(UART_BASE_VALID(base));

    *UART_CONTROL_3(base) |= SA1100_UART_RX_FIFO_INT_ENABLE_MASK;
}


static int
uart_irq_disable(void *base)
{
    int enable;

    BSP_ASSERT(UART_BASE_VALID(base));

    enable = *UART_CONTROL_3(base);

    *UART_CONTROL_3(base) = enable & ~SA1100_UART_RX_FIFO_INT_ENABLE_MASK;

    return (enable & SA1100_UART_RX_FIFO_INT_ENABLE_MASK);
}

static void uart_putchar(void *base, char c)
{
    BSP_ASSERT(UART_BASE_VALID(base));

    while ((*UART_STATUS_1(base) & SA1100_UART_TX_FIFO_NOT_FULL) == 0)
        ;

    *UART_DATA(base) = c & SA1100_UART_DATA_MASK;
}


static void
uart_write(void *base, const char *buf, int len)
{
    BSP_ASSERT(UART_BASE_VALID(base));

    while (len-- > 0)
        uart_putchar(base, *buf++);
}

static int
uart_rcvchar(void *base)
{
    BSP_ASSERT(UART_BASE_VALID(base));

    while ((*UART_STATUS_1(base) & SA1100_UART_RX_FIFO_NOT_EMPTY) == 0)
        ;
    return(*UART_DATA(base) & SA1100_UART_DATA_MASK);
}

static int
uart_getchar(void *base)
{
    int c, was_enabled;

    BSP_ASSERT(UART_BASE_VALID(base));

    was_enabled = uart_irq_disable(base);
    if ((c = _bsp_dequeue(&uart_queue[UART_INDEX(base)])) < 0)
        c = uart_rcvchar(base);
    if (was_enabled)
	uart_irq_enable(base);

#if DEBUG_LOCAL_ECHO
    uart_putchar(base, c);
#endif
    return c;
}

static int
uart_read(void *base, char *buf, int len)
{
    int ch;

    BSP_ASSERT(UART_BASE_VALID(base));

    if (len <= 0)
        return 0;

    ch = uart_getchar(base);
    *buf = ch;
    return 1;
}

static int
uart_irq_handler(int irq_nr, void *regs)
{
    unsigned char ch;
    int return_code = 0;

#if DEBUG_UART_CODE
    bsp_printf("In uart_irq_handler(%d, 0x%08lx)\n", irq_nr, regs);
    _bsp_dump_regs(regs);
    bsp_printf("uart_dbg_base == 0x%08lx\n", uart_dbg_base);
    bsp_printf("*UART_CONTROL_0(UART_BASE_0) = 0x%08lx\n", *UART_CONTROL_0(UART_BASE_0));
    bsp_printf("*UART_CONTROL_1(UART_BASE_0) = 0x%08lx\n", *UART_CONTROL_1(UART_BASE_0));
    bsp_printf("*UART_CONTROL_2(UART_BASE_0) = 0x%08lx\n", *UART_CONTROL_2(UART_BASE_0));
    bsp_printf("*UART_CONTROL_3(UART_BASE_0) = 0x%08lx\n", *UART_CONTROL_3(UART_BASE_0));
    bsp_printf("*UART_STATUS_0(UART_BASE_0)  = 0x%08lx\n", *UART_STATUS_0(UART_BASE_0));
    bsp_printf("*UART_STATUS_1(UART_BASE_0)  = 0x%08lx\n", *UART_STATUS_1(UART_BASE_0));

    bsp_printf("*UART_CONTROL_0(UART_BASE_1) = 0x%08lx\n", *UART_CONTROL_0(UART_BASE_1));
    bsp_printf("*UART_CONTROL_1(UART_BASE_1) = 0x%08lx\n", *UART_CONTROL_1(UART_BASE_1));
    bsp_printf("*UART_CONTROL_2(UART_BASE_1) = 0x%08lx\n", *UART_CONTROL_2(UART_BASE_1));
    bsp_printf("*UART_CONTROL_3(UART_BASE_1) = 0x%08lx\n", *UART_CONTROL_3(UART_BASE_1));
    bsp_printf("*UART_STATUS_0(UART_BASE_1)  = 0x%08lx\n", *UART_STATUS_0(UART_BASE_1));
    bsp_printf("*UART_STATUS_1(UART_BASE_1)  = 0x%08lx\n", *UART_STATUS_1(UART_BASE_1));
#endif /* DEBUG_UART_CODE */

    if (((*UART_STATUS_0(uart_dbg_base)) &
         (SA1100_UART_RX_SERVICE_REQUEST | SA1100_UART_RX_IDLE)) != 0)
    {
        BSP_ASSERT(UART_BASE_VALID(uart_dbg_base));

        ch = *UART_DATA(uart_dbg_base) & SA1100_UART_DATA_MASK;

        /*
         * Clear the UART Status bits by writing a 1
         */
        *UART_STATUS_0(uart_dbg_base) = (SA1100_UART_RX_SERVICE_REQUEST | SA1100_UART_RX_IDLE);

        /*
         * If we read a Ctrl-C then we need to let
         * GDB handle it.  Return 0 to have the GDB
         * exception processor run.
         */
        if (ch == '\003')
        {
            return_code = 0;
        } else {
            _bsp_enqueue(&uart_queue[UART_INDEX(uart_dbg_base)], ch);
#if DEBUG_LOCAL_ECHO
            uart_putchar(uart_dbg_base, ch);
#endif
            return_code = 1;
        }
    }

    return return_code;
}

static int
uart_setbaud(void *base, int bps)
{
    int   i;
    const struct _baud *bp;

    BSP_ASSERT(UART_BASE_VALID(base));
    if (uart_baud[UART_INDEX(base)] != bps)
    {
        for (i = 0, bp = bauds; i < (sizeof(bauds)/sizeof(bauds[0])); i++, bp++)
        {
            if (bp->baud == bps)
            {
                unsigned long old_control_3 = *UART_CONTROL_3(base);

                /*
                 * Must disable the UART to write to the Baud Rate Divisors
                 */
                *UART_CONTROL_3(base) &= ~(SA1100_UART_RX_ENABLE_MASK |
                                           SA1100_UART_TX_ENABLE_MASK);
                
                /*
                 * Setup the UART Baud Rate Divisor
                 */
                *UART_CONTROL_1(base) = bp->divisor_high & SA1100_UART_H_BAUD_RATE_DIVISOR_MASK;
                *UART_CONTROL_2(base) = bp->divisor_low  & SA1100_UART_L_BAUD_RATE_DIVISOR_MASK;

                /*
                 * Restore the state of the UART
                 */
                *UART_CONTROL_3(base) = old_control_3;

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

    BSP_ASSERT(UART_BASE_VALID(base));

    va_start (ap, func);

    switch (func) {
    case COMMCTL_SETBAUD:
        i = va_arg(ap, int);
        retval = uart_setbaud((void *)base, i);
        break;

    case COMMCTL_GETBAUD:
        retval = uart_baud[UART_INDEX(uart_dbg_base)];
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

    return retval;
}

#if DEBUG_UART_FUNCTIONS
#else
static
#endif /* DEBUG_UART_FUNCTIONS */
void
init_uart(void *base, int bps)
{
    BSP_ASSERT(UART_BASE_VALID(base));

    if (base == UART_BASE_1)
    {
#ifdef __BOARD_SA1110AS__
	/* This UART is multiplexed with GPCLK. Need to select UART */
	*SA1110_GPCLK_CONTROL_0 = SA1110_GPCLK_SUS_UART;
#else
        /*
         * This is the UART that is multiplexed with the
         * SDLC controller.
         *
         * We need to reassign GPIO pins 14 and 15 to the
         * UART function of the SDLC UART.
         */
        *SA1100_PPC_PIN_ASSIGNMENT |= SA1100_PPC_UART_PIN_REASSIGNED;
        *SA1100_GPIO_ALTERNATE_FUNCTION |= (SA1100_GPIO_PIN_14 | SA1100_GPIO_PIN_15);
        *SA1100_GPIO_PIN_DIR_REGISTER |= SA1100_GPIO_PIN_14;
        *SA1100_GPIO_PIN_DIR_REGISTER &= ~SA1100_GPIO_PIN_15;
#endif
    }

    /*
     * Reset the Status Bits
     */
    *UART_STATUS_0(base) = ~0;

    /*
     * Set the data info
     */
    *UART_CONTROL_0(base) = SA1100_UART_PARITY_DISABLED       |
                            SA1100_UART_STOP_BITS_1           |
                            SA1100_UART_DATA_BITS_8           |
                            SA1100_UART_SAMPLE_CLOCK_DISABLED;

    /*
     * Set the baud rate
     */
    uart_setbaud((void *)base, bps);

    /*
     * Turn everything on
     */
    *UART_CONTROL_3(base) = SA1100_UART_RX_ENABLED           |
                            SA1100_UART_TX_ENABLED           |
                            SA1100_UART_BREAK_DISABLED       |
                            SA1100_UART_RX_FIFO_INT_DISABLED |
                            SA1100_UART_TX_FIFO_INT_DISABLED |
                            SA1100_UART_NORMAL_OPERATION;
}

/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_sa1100_comm(void)
{
    int i, qi;
    void *base;

    for (i = 0; i < BSP_NUM_COMMS; i++) {
	base = _bsp_comm_list[i].procs.ch_data;
	qi = UART_INDEX(base);
	_bsp_queue_init(&uart_queue[qi], uart_buffer[qi], sizeof(uart_buffer[qi]));
	init_uart(base, 38400);
    }
}


/* #define FLUSH_WHOLE_CACHE */

/*
 *  Flush Icache. Entire cache -- SA1100 does not support entry flushing.
 */
void __icache_flush(void *__p, int __nbytes)
{
    __mcr(ARM_CACHE_COPROCESSOR_NUM,
          ARM_COPROCESSOR_OPCODE_DONT_CARE,
          0,
          ARM_CACHE_OPERATIONS_REGISTER,
          ARM_FLUSH_CACHE_INST_RM,
          ARM_FLUSH_CACHE_INST_OPCODE);
}

#define DEBUG_DCACHE_FLUSH 0
#if DEBUG_DCACHE_FLUSH
#define FP_DUMP()                                                 \
{                                                                 \
    unsigned long *fp = (unsigned long *)__get_fp();              \
                                                                  \
    bsp_printf("\nfp = 0x%08lx\n", fp);                           \
    bsp_printf("*(fp - 8) = 0x%08lx\n", *(fp - 8));               \
    bsp_printf("*(fp - 7) = 0x%08lx\n", *(fp - 7));               \
    bsp_printf("*(fp - 6) = 0x%08lx\n", *(fp - 6));               \
    bsp_printf("*(fp - 5) = 0x%08lx\n", *(fp - 5));               \
    bsp_printf("*(fp - 4) = 0x%08lx\n", *(fp - 4));               \
    bsp_printf("*(fp - 3) = 0x%08lx\n", *(fp - 3));               \
    bsp_printf("*(fp - 2) = 0x%08lx\n", *(fp - 2));               \
    bsp_printf("*(fp - 1) = 0x%08lx\n", *(fp - 1));               \
                                                                  \
    bsp_printf("*(fp + 0) = 0x%08lx\n", *(fp + 0));               \
                                                                  \
    bsp_printf("*(fp + 1) = 0x%08lx\n", *(fp + 1));               \
    bsp_printf("*(fp + 2) = 0x%08lx\n", *(fp + 2));               \
    bsp_printf("*(fp + 3) = 0x%08lx\n", *(fp + 3));               \
    bsp_printf("*(fp + 4) = 0x%08lx\n", *(fp + 4));               \
    bsp_printf("*(fp + 5) = 0x%08lx\n", *(fp + 5));               \
    bsp_printf("*(fp + 6) = 0x%08lx\n", *(fp + 6));               \
    bsp_printf("*(fp + 7) = 0x%08lx\n", *(fp + 7));               \
    bsp_printf("*(fp + 8) = 0x%08lx\n", *(fp + 8));               \
}
#endif /* DEBUG_DCACHE_FLUSH */

/*
 *  Flush entire Dcache.
 */
void __dcache_flush(void *__p, int __nbytes)
{
#if DEBUG_DCACHE_FLUSH
    volatile int i, j[30];

    for (i = 0; i < 30; i++)
        j[i] = 0xCAFEBABE;

    for (i = 0; i < 30; i++)
        bsp_printf("j[%d] = 0x%08lx\n", i, j[i]);

    FP_DUMP();
#endif /* DEBUG_DCACHE_FLUSH */

#ifdef FLUSH_WHOLE_CACHE
    {
        register volatile unsigned long *ptr = REG32_PTR(SA1100_ZEROS_BANK_BASE);
        register volatile unsigned long *ptr_stop = REG32_PTR(SA1100_ZEROS_BANK_BASE +
                                                              SA1100_DCACHE_SIZE);
        register unsigned long temp;

        /*
         * Do a load to each line of the cache to
         * force a write-back
         */
        while (ptr < ptr_stop)
        {
            temp = *ptr;
            ptr += SA1100_DCACHE_LINESIZE_WORDS;
        }

        /*
         * Now invalidate the cache
         */
        __mcr(ARM_CACHE_COPROCESSOR_NUM,
              ARM_COPROCESSOR_OPCODE_DONT_CARE,
              temp,
              ARM_CACHE_OPERATIONS_REGISTER,
              ARM_FLUSH_CACHE_DATA_RM,
              ARM_FLUSH_CACHE_DATA_OPCODE);
    }
#else /* FLUSH_WHOLE_CACHE */
    {
        register volatile unsigned long *base = SA1100_DCACHE_LINE_BASE(__p);
        register volatile unsigned long *ptr_stop = SA1100_DCACHE_LINE_BASE(__p + __nbytes);

        do
        {
            /*
             * Clean each cache line
             */
            __mcr(ARM_CACHE_COPROCESSOR_NUM,
                  ARM_COPROCESSOR_OPCODE_DONT_CARE,
                  base,
                  ARM_CACHE_OPERATIONS_REGISTER,
                  ARM_CLEAN_CACHE_DATA_ENTRY_RM,
                  ARM_CLEAN_CACHE_DATA_ENTRY_OPCODE);

            /*
             * Flush each cache line
             */
            __mcr(ARM_CACHE_COPROCESSOR_NUM,
                  ARM_COPROCESSOR_OPCODE_DONT_CARE,
                  base,
                  ARM_CACHE_OPERATIONS_REGISTER,
                  ARM_FLUSH_CACHE_DATA_SINGLE_RM,
                  ARM_FLUSH_CACHE_DATA_SINGLE_OPCODE);

            base += SA1100_DCACHE_LINESIZE_WORDS;
        } while (base <= ptr_stop);
    }
#endif /* FLUSH_WHOLE_CACHE */

#if DEBUG_DCACHE_FLUSH
    FP_DUMP();

    for (i = 0; i < 30; i++)
        bsp_printf("j[%d] = 0x%08lx\n", i, j[i]);

    while (1) ;
#endif /* DEBUG_DCACHE_FLUSH */
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
