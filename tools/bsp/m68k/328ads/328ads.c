/*
 * 328ads.c -- Support for Motorola Dragonball(68328) ADS board.
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
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = sizeof(_bsp_memory_list)/sizeof(_bsp_memory_list[0]);

/*
 * Toggle Port Pin PJ3 for debugging purposes.
 *
 * This pin is not hooked up to an LED so it must be probed with
 * a logic probe or similar device.
 *
 * This pin appears in jumper block P8, pin 8
 */
void flash_led(int n, int which)
{
    /*
     * Setup PJ3 as GPIO output
     */
    *MC68328_PJDIR  |=  MC68328_PJ3;
    *MC68328_PJSEL  |=  MC68328_PJ3;
  
    /*
     * Double n so we turn on and off for each count
     */
    n <<= 1;
    
    /*
     * Toggle a Port PIN n times
     */
    while(n-- > 0) {
        int i;
        *MC68328_PJDATA ^= MC68328_PJ3;
        i = 200000; while (--i);
    }
}

/*
 * Enable/Disable separate debug functions
 */
#define DEBUG_UART_CODE      0
#define DEBUG_UART_FUNCTIONS 0
#define DEBUG_LOCAL_ECHO     0

/*
 * Macros to make finding the registers easier
 */
#define UART_INDEX(base)                 (((base) == UART_BASE_0) ? 0 : 1)
#define UART_BASE_VALID(base)            (((base) == UART_BASE_0) || ((base) == UART_BASE_1))
#define UART_MODE_REGISTER_1(base)       ((volatile unsigned char*)((base) + MC68681_MR1A_o))
#define UART_MODE_REGISTER_2(base)       ((volatile unsigned char*)((base) + MC68681_MR2A_o))
#define UART_STATUS_REGISTER(base)       ((volatile unsigned char*)((base) + MC68681_SRA_o))
#define UART_CLKSEL_REGISTER_WRITE(base) ((volatile unsigned char*)((base) + MC68681_CSRA_WRITE_o))
#define UART_CLKSEL_REGISTER_READ(base)  ((volatile unsigned char*)((base) + MC68681_CSRA_READ_o))
#define UART_COMMAND_REGISTER(base)      ((volatile unsigned char*)((base) + MC68681_CRA_o))
#define UART_RECEIVE_REGISTER(base)      ((volatile unsigned char*)((base) + MC68681_RBA_o))
#define UART_TRANSMIT_REGISTER(base)     ((volatile unsigned char*)((base) + MC68681_TBA_o))

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
	{ "duart PortA", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE_0, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    },
    {
	{ "duart PortB", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_BASE_1, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
};
int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);

/*
 * Baud rate selection stuff
 *
 * CAREFUL: The baudgroup is shared between the ports,
 * so if you do something on one port which modifies
 * the baudgroup, the other port will likely be affected.
 */
static int uart_baud[BSP_NUM_COMMS] = {-1,  -1};
static int uart_baudgroup[BSP_NUM_COMMS] = {-1,  -1};

struct _baud {
    int baud;
    unsigned char clock_select_value;
    unsigned char baud_rate_group;
};

const static struct _baud bauds[] = {
    {   50, 0x00, 1},
    {   75, 0x00, 2},
    {  110, 0x11, 1},
    {  134, 0x22, 1},
    {  150, 0x33, 2},
    {  200, 0x33, 1},
    {  300, 0x44, 1},
    {  600, 0x55, 1},
    { 1050, 0x77, 1},
    { 1200, 0x66, 1},
    { 1800, 0xAA, 2},
    { 2000, 0x77, 2},
    { 2400, 0x88, 1},
    { 4800, 0x99, 1},
    { 7200, 0xAA, 1},
    { 9600, 0xBB, 1},
    {19200, 0xCC, 2},
    {38400, 0xCC, 1},
};

/*
 * UART queuing stuff
 */
#define UART_BUFSIZE 256
static bsp_queue_t uart_queue[BSP_NUM_COMMS];
static char uart_buffer[BSP_NUM_COMMS][UART_BUFSIZE];

/*
 * The currently selected debug port
 */
static volatile unsigned char *uart_dbg_base = NULL;

/*
 * Board specific data for the 328ADS board.
 * (currently the shadow register for the DUART)
 */
static ads328_board_data board_data;

static void
uart_irq_enable(void *base)
{
    if (UART_BASE_VALID(base))
    {
        if (UART_INDEX(base) == 0) {
            board_data.mc68681_imr_shadow |= MC68681_INT_RXRDYA;
        } else if (UART_INDEX(base) == 1) {
            board_data.mc68681_imr_shadow |= MC68681_INT_RXRDYB;
        }
        *MC68681_IMR = board_data.mc68681_imr_shadow;
    }
}


static void
uart_irq_disable(void *base)
{
    if (UART_BASE_VALID(base))
    {
        if (UART_INDEX(base) == 0) {
            board_data.mc68681_imr_shadow &= ~MC68681_INT_RXRDYA;
        } else {
            board_data.mc68681_imr_shadow &= ~MC68681_INT_RXRDYB;
        }
        *MC68681_IMR = board_data.mc68681_imr_shadow;
    }
}


static void
uart_putchar(void *base, char c)
{
    if (UART_BASE_VALID(base))
    {
        while ((MC68681_SR_TXRDY & *UART_STATUS_REGISTER(base)) == 0)
            ;
        *UART_TRANSMIT_REGISTER(base) = c;
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
        while ((MC68681_SR_RXRDY & *UART_STATUS_REGISTER(base)) == 0)
            ;
        return(*UART_RECEIVE_REGISTER(base));
    }

    return(-1);
}

static int
uart_getchar(void *base)
{
    int c;

    if (UART_BASE_VALID(base))
    {
        uart_irq_disable(base);
        if ((c = _bsp_dequeue(&uart_queue[UART_INDEX(base)])) < 0)
            c = uart_rcvchar(base);
        uart_irq_enable(base);

#if DEBUG_LOCAL_ECHO
        uart_putchar(base, ch);
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
    volatile unsigned char *port;
    int return_code = 0;

#if DEBUG_UART_CODE
    bsp_printf("In uart_irq_handler(%d, 0x%08lx)\n", irq_nr, regs);
    _bsp_dump_regs(regs);
    bsp_printf("MC68681_SRA       = 0x%02x\n", *MC68681_SRA);
    bsp_printf("MC68681_MR1A      = 0x%02x\n", *MC68681_MR1A);
    bsp_printf("MC68681_SRA       = 0x%02x\n", *MC68681_SRA);
    bsp_printf("MC68681_CSRA_READ = 0x%02x\n", *MC68681_CSRA_READ);
#if 0
    bsp_printf("MC68681_RBA       = 0x%02x\n", *MC68681_RBA);
#endif
    bsp_printf("MC68681_IPCR      = 0x%02x\n", *MC68681_IPCR);
    bsp_printf("MC68681_ISR       = 0x%02x\n", *MC68681_ISR);
    bsp_printf("MC68681_CUR       = 0x%02x\n", *MC68681_CUR);
    bsp_printf("MC68681_CLR       = 0x%02x\n", *MC68681_CLR);
    bsp_printf("MC68681_IVR       = 0x%02x\n", *MC68681_IVR);
    bsp_printf("MC68681_IP        = 0x%02x\n", *MC68681_IP);
    bsp_printf("IMR = 0x%08lx\n", *MC68328_IMR);
    bsp_printf("ISR = 0x%08lx\n", *MC68328_ISR);
    bsp_printf("IPR = 0x%08lx\n", *MC68328_IPR);
#endif /* DEBUG_UART_CODE */
  
    if ((uart_dbg_base == UART_BASE_0) &&
        ((MC68681_SR_RXRDY & *UART_STATUS_REGISTER(UART_BASE_0)) != 0)) {
        /*
         * Expected interrupt on Port 0
         */
        port = UART_BASE_0;
        return_code = 1;
    } else if ((uart_dbg_base == UART_BASE_1) &&
               ((MC68681_SR_RXRDY & *UART_STATUS_REGISTER(UART_BASE_1)) != 0)) {
        /*
         * Expected interrupt on Port 1
         */
        port = UART_BASE_1;
        return_code = 1;
    } else {
        /*
         * No data ready.  Not my interrupt
         */
        return_code = 0;
    }
  
    if (return_code != 0)
    {
        while ((MC68681_SR_RXRDY & *UART_STATUS_REGISTER(port)) != 0)
        {
            ch = *UART_RECEIVE_REGISTER(port);

            /*
             * If we read a Ctrl-C then we need to let
             * GDB handle it.  Return 0 to have the GDB
             * exception processor run.
             */
            if (ch == '\003')
            {
                return_code = 0;
                break;
            }
    
            _bsp_enqueue(&uart_queue[UART_INDEX(port)], ch);
#if DEBUG_LOCAL_ECHO
            uart_putchar(port, ch);
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
  
    if (UART_BASE_VALID(base))
    {
        for (i = 0, bp = bauds; i < (sizeof(bauds)/sizeof(bauds[0])); i++, bp++)
        {
            if (bp->baud == bps)
            {
                if (bp->baud_rate_group != uart_baudgroup[UART_INDEX(base)])
                {
                    /*
                     * We need to change the baud group
                     */
                    *UART_COMMAND_REGISTER(base) = MC68681_CMD_RST_RX | MC68681_CMD_RX_DISABLED;
                    *UART_COMMAND_REGISTER(base) = MC68681_CMD_RST_TX | MC68681_CMD_TX_DISABLED;

                    if (bp->baud_rate_group == 1)
                        *MC68681_ACR &= ~MC68681_AUX_BRG_SELECT;
                    else
                        *MC68681_ACR |= MC68681_AUX_BRG_SELECT;
                    uart_baudgroup[UART_INDEX(base)] = bp->baud_rate_group;

                    *UART_COMMAND_REGISTER(base) = MC68681_CMD_RX_ENABLED;
                    *UART_COMMAND_REGISTER(base) = MC68681_CMD_TX_ENABLED;
                }

                *UART_CLKSEL_REGISTER_WRITE(base) = bp->clock_select_value;
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
                    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_68328_IRQ_IRQ1,
                                    BSP_VEC_REPLACE, &uart_vec);
                    bsp_enable_irq(BSP_68328_IRQ_IRQ1);
                    uart_irq_enable(base);
                }
                uart_dbg_base = base;
            }
            break;
	
        case COMMCTL_REMOVE_DBG_ISR:
            uart_irq_disable(base);
            bsp_disable_irq(BSP_68328_IRQ_IRQ1);
            bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_68328_IRQ_IRQ1, &uart_vec);
            uart_dbg_base = NULL;
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
    if (UART_BASE_VALID(base))
    {
        uart_setbaud((void *)base, bps);

        /*
         * Reset the Channel Mode Pointer to point to Register 1
         */
        *UART_COMMAND_REGISTER(base) = MC68681_CMD_RST_MR_PTR;
  
        /*
         * Initialize DUART Mode Register 1
         *
         * 0x13 = 00010011
         *        ||||||++--> Bits per Character = 8 
         *        |||||+----> Parity Type        = Don't Care
         *        |||++-----> Parity Mode        = No Parity
         *        ||+-------> Error Mode         = Character
         *        |+--------> Rx IRQ Select      = RxRDY
         *        +---------> Rx RTR Control     = Disabled
         */
        *UART_MODE_REGISTER_1(base) = 0x13;
  
        /*
         * Now the Mode Register pointer points to Register 2
         *
         * 0x00 = 00000000
         *        ||||||++--> unused
         *        ||||++----> Stop Bit Length        = 1 Stop Bit
         *        |||+------> CTS-Enable Transmitter = Disabled
         *        ||+-------> TX RTS Control         = Disabled
         *        ++--------> Channel Mode           = Normal
         */
        *UART_MODE_REGISTER_2(base) = 0x00;
  
        /*
         * Initialize the Interrupt Status Register
         *
         * 0x00 = 00000000
         *        |||||||+--> TxRDYA Interrupt Disable
         *        ||||||+---> RxRDYA/FFULLA Interrupt Disable
         *        |||||+----> Delta Break A Interrupt Disable
         *        ||||+-----> Counter/Timer Ready Interrupt Disable
         *        |||+------> TxRDYB Interrupt Disable
         *        ||+-------> RxRDYB/FFULLB Interrupt Disable
         *        |+--------> Delta Break A Interrupt Disable
         *        +---------> Input Port Change Interrupt Disable
         */
        *MC68681_ISR = 0x00;
  
        /*
         * Enable the TX/RX blocks.
         */
        *UART_COMMAND_REGISTER(base) = MC68681_CMD_TX_ENABLED | MC68681_CMD_RX_ENABLED;

        /*
         * Initialize to produce no interrupts
         */
        board_data.mc68681_imr_shadow = 0;
        *MC68681_IMR = board_data.mc68681_imr_shadow;

        /*
         * Setup our vector to be Autovector 1
         */
        *MC68681_IVR = BSP_CORE_EXC_LEVEL_1_AUTO;
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
}

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    /*
     * Install board specific data
     */
    bsp_shared_data->__board_data = &board_data;

    /*
     * Override default platform info.
     */
    _bsp_platform_info.cpu = "M68K";
    _bsp_platform_info.board = "MC68328(Dragonball) ADS";
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

