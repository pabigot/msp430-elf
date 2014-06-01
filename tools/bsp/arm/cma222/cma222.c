/*
 * cma222.c -- Support for Cogent ARM(R) CMA222/CMA110 Evaluation Board
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include <stdlib.h>
#include <bsp/cma222.h>
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

/*
 * Toggle LED for debugging purposes.
 */
void flash_led(int n, int which)
{
}

/*---------------------------------------------------------------------------*/
/* Generic defines                                                           */

#define MBD_BASE                0x00000000
#define ENDIAN                  0x00


/*---------------------------------------------------------------------------
 * Cogent board specific 16552 code.
 ---------------------------------------------------------------------------*/

#define SIO_BASE_A 		(unsigned char *)(MBD_BASE + 0xE900040) /* channel A */
#define SIO_BASE_B		(unsigned char *)(MBD_BASE + 0xE900000) /* channel B */

/* register offsets */
#define SIO_RXD			0x00 + ENDIAN	/* receive data, read, dlab = 0 */
#define SIO_TXD			0x00 + ENDIAN	/* transmit data, write, dlab = 0 */
#define	SIO_BAUDLO		0x00 + ENDIAN	/* baud divisor 0-7, read/write, dlab = 1 */
#define	SIO_IEN			0x08 + ENDIAN	/* interrupt enable, read/write, dlab = 0 */
#define	SIO_BAUDHI		0x08 + ENDIAN	/* baud divisor 8-15, read/write, dlab = 1 */
#define	SIO_ISTAT		0x10 + ENDIAN	/* interrupt status, read, dlab = 0 */
#define	SIO_FCTL		0x10 + ENDIAN	/* fifo control, write, dlab = 0 */
#define	SIO_AFR			0x10 + ENDIAN	/* alternate function register, read/write, dlab = 1 */
#define	SIO_LCTL		0x18 + ENDIAN	/* line control, read/write */
#define	SIO_MCTL		0x20 + ENDIAN	/* modem control read/write */
#define	SIO_LSTAT		0x28 + ENDIAN	/* line status read */
#define	SIO_MSTAT		0x30 + ENDIAN	/* modem status read */
#define	SIO_SPR			0x38 + ENDIAN	/* scratch pad register */

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

#define UART_BUFSIZE 32

static bsp_queue_t uart1_queue;
static bsp_queue_t uart2_queue;
static char uart1_buffer[UART_BUFSIZE];
static char uart2_buffer[UART_BUFSIZE];

static void
uart_irq_enable(void *base)
{
    *(volatile unsigned char *)(base + SIO_IEN) = 5;
}

static int
uart_irq_disable(void *base)
{
    int was_enabled;
    volatile unsigned char *tty_ien;

    tty_ien = (volatile unsigned char *)base + SIO_IEN;

    was_enabled = *tty_ien & 5;
    *tty_ien = 0;

    return was_enabled != 0;
}


static void
uart_putchar(void *base, char c)
{
    volatile unsigned char *tty_status;
    volatile unsigned char *tty_tx;
    int was_enabled;

    tty_status = (volatile unsigned char *)base + SIO_LSTAT;
    tty_tx     = (volatile unsigned char *)base + SIO_TXD;

    was_enabled = uart_irq_disable(base);

    while(!(*tty_status & SIO_LSTAT_TEMTY))
	;
	
    *tty_tx = c;

    if (was_enabled)
	uart_irq_enable(base);
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
    volatile unsigned char *tty_status;
    volatile unsigned char *tty_rx;
    unsigned char c;


    tty_status = (volatile unsigned char *)base + SIO_LSTAT;
    tty_rx     = (volatile unsigned char *)base + SIO_RXD;

    if (block)
	while((*tty_status & SIO_LSTAT_RRDY) == 0)
	    ;

    if (*tty_status & SIO_LSTAT_RRDY)
	c = *tty_rx;
    else
	return -1;

    return c;
}


static int
uart_getchar(void *base)
{
    int ch, was_enabled;

    was_enabled = uart_irq_disable(base);
    ch = _bsp_dequeue(base == SIO_BASE_A ? &uart1_queue : &uart2_queue);
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
    ch = _bsp_dequeue(base == SIO_BASE_A ? &uart1_queue : &uart2_queue);
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


static volatile unsigned char *uart_dbg_base;

static int
uart_irq_handler(int irq_nr, void *regs)
{
    volatile unsigned char *tty_istat = uart_dbg_base + SIO_ISTAT;
    int   ch, rval = 1;
    unsigned char istat, lstat;

    istat = *tty_istat; /* clear pending irq */
    istat &= 0x0f;      /* mask off fifo bits */

    if (istat == 6) {
	lstat = *(uart_dbg_base + SIO_LSTAT);
    } else if (istat == 4 || istat == 12) {
	ch = uart_rcvchar((void*)uart_dbg_base, 1);

	if (ch == '\003')
	    rval = 0;
	else
	    _bsp_enqueue(uart_dbg_base == SIO_BASE_A ?
			 &uart1_queue : &uart2_queue, ch);
    } else if ((istat & 1) == 0)
	bsp_printf("unexpected uart irq: istat<0x%x> lsr<0x%x>\n",
		   istat, *(uart_dbg_base + SIO_LSTAT));

    return rval;
}


static int uartA_baud, uartB_baud;

struct _baud {
    int baud;
    unsigned char msb;
    unsigned char lsb;
};

const static struct _baud bauds[] = {
    { 1200,  0,192 },
    { 2400,  0, 96 },
    { 4800,  0, 48 },
    { 9600,  0, 24 },
    { 19200, 0, 12 },
    { 38400, 0,  6 }
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
	    if (base == SIO_BASE_A) {
		uartA_baud = bps;
	    }
	    else
		uartB_baud = bps;


	    return 0;
	}
    return -1;
}


static void
init_uart(unsigned char *base, int bps)
{
    unsigned char dummy;

    uart_setbaud(base, bps);

    /* clear pending irq */
    dummy = *(unsigned char *)(base + SIO_ISTAT);

    /* clear modem status */
    dummy = *(unsigned char *)(base + SIO_MSTAT);

    /* enable FIFO */
    *(unsigned char *)(base + SIO_FCTL) = 7;

    /* assert DTR and RTS, enable interrup line */
    *(base + SIO_MCTL) = 0x0b;
}


static int
uart_control(void *base, int func, ...)
{
    int          i, irq_nr, retval = 0;
    va_list      ap;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    if (base == SIO_BASE_A)
	irq_nr = CMA_IRQ_SERIAL0;
    else
	irq_nr = CMA_IRQ_SERIAL1;

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(base, i);
	break;

      case COMMCTL_GETBAUD:
	retval = (base == SIO_BASE_A) ? uartA_baud : uartB_baud;
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
	{ (void*)SIO_BASE_A, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    },
    {
	{ "uart2", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)SIO_BASE_B, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
};


int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);


/*
 *  Cogent board specific LCD code
 */

/***************************************
 * FEMA 162B 16 character x 2 line LCD *
 * base addresses and register offsets *
 ***************************************/
#define LCD_DLY			500	/* timeout value for lcd_stat */

#define LCD_BASE 	        (unsigned char *)(MBD_BASE + 0xEB00000)

#define LCD_DATA		0x00 + ENDIAN /* read/write lcd data */
#define LCD_STAT		0x08 + ENDIAN /* read lcd busy status */
#define LCD_CMD			0x08 + ENDIAN /* write lcd command */

/* status register bit definitions */
#define LCD_STAT_BUSY	0x80	/* 1 = display busy */
#define LCD_STAT_ADD	0x7F	/* bits 0-6 return current display address */

/* command register definitions */
#define LCD_CMD_RST		0x01	/* clear entire display and reset display address */
#define LCD_CMD_HOME	        0x02	/* reset display address and reset any shifting */
#define LCD_CMD_ECL		0x04	/* move cursor left one position on next data write */
#define LCD_CMD_ESL		0x05	/* shift display left one position on next data write */
#define LCD_CMD_ECR		0x06	/* move cursor right one position on next data write */
#define LCD_CMD_ESR		0x07	/* shift display right one position on next data write */
#define LCD_CMD_DOFF	        0x08	/* display off, cursor off, blinking off */
#define LCD_CMD_BL		0x09	/* blink character at current cursor position */
#define LCD_CMD_CUR		0x0A	/* enable cursor on */
#define LCD_CMD_DON		0x0C	/* turn display on */
#define LCD_CMD_CL		0x10	/* move cursor left one position */
#define LCD_CMD_SL		0x14	/* shift display left one position */
#define LCD_CMD_CR		0x18	/* move cursor right one position */
#define LCD_CMD_SR		0x1C	/* shift display right one position */
#define LCD_CMD_MODE	        0x38	/* sets 8 bits, 2 lines, 5x7 characters */
#define LCD_CMD_ACG		0x40	/* bits 0-5 sets the character generator address */
#define LCD_CMD_ADD		0x80	/* bits 0-6 sets the display data address to line 1 + */

/* LCD status values */
#define LCD_OK			0x00
#define LCD_ERR			0x01

#define LCD_LINE0       0x00
#define LCD_LINE1       0x40

#define LCD_LINE_LENGTH 16

static char lcd_line0[LCD_LINE_LENGTH+1];
static char lcd_line1[LCD_LINE_LENGTH+1];
static char *lcd_line[2] = { lcd_line0, lcd_line1 };
static int lcd_curline = 0;
static int lcd_linepos = 0;

static void lcd_dis(int add, char *string);

static void
init_lcd(void)
{
    volatile unsigned char *lcd = LCD_BASE;
    int i, x;
    
    /* wait for not busy */
    while (*(lcd + LCD_STAT) & LCD_STAT_BUSY)
        for (x = 0; x < LCD_DLY; x++);

    /* configure the lcd for 8 bits/char, 2 lines */
    /* and 5x7 dot matrix */
    *(lcd + LCD_CMD) = LCD_CMD_MODE;
	
    /* delay */
    for (x = 0; x < LCD_DLY; x++){}

    /* wait for not busy */
    while (*(lcd + LCD_STAT) & LCD_STAT_BUSY)
    {
        /* delay */
        for (x = 0; x < LCD_DLY; x++){}
    }

    /* turn the LCD display on */
    *(lcd + LCD_CMD) = LCD_CMD_DON;

    /* delay */
    for (x = 0; x < LCD_DLY; x++){}

    lcd_curline = 0;
    lcd_linepos = 0;

    for( i = 0; i < LCD_LINE_LENGTH; i++ )
        lcd_line[0][i] = lcd_line[1][i] = ' ';

    lcd_line[0][LCD_LINE_LENGTH] = lcd_line[1][LCD_LINE_LENGTH] = 0;

    lcd_dis( LCD_LINE0, lcd_line[0] );
    lcd_dis( LCD_LINE1, lcd_line[1] );
}

/* this routine writes the string to the LCD */
/* display after setting the address to add */
static void
lcd_dis(int add, char *string)
{
    unsigned char *lcd = LCD_BASE;    /* pointer to the lcd */
    int i;

    /* write the string out to the display */
    /* stopping when we reach 0 */
    for (i = 0; *string != '\0'; i++)
    {
	int x;

        /* wait for not busy */
        while (*(lcd + LCD_STAT) & LCD_STAT_BUSY)
        {
            /* delay */
            for (x = 0; x < LCD_DLY; x++){}
        }

        /* write the address */
        *(lcd + LCD_CMD) = LCD_CMD_ADD + add;
        add++;

        /* delay */
        for (x = 0; x < LCD_DLY; x++){}

        /* wait for not busy */
        while (*(lcd + LCD_STAT) & LCD_STAT_BUSY)
        {
            /* delay */
            for (x = 0; x < LCD_DLY; x++){}
        }

        /* write the data */
        *(lcd + LCD_DATA) = *string++;

        /* delay */
        for (x = 0; x < LCD_DLY; x++){}
    }
}


void
__lcd_outbyte(char c)
{
    int i;
    
    // Truncate long lines
    if( lcd_linepos >= LCD_LINE_LENGTH ) return;

    // ignore CR
    if( c == '\r' ) return;
    
    if( c == '\n' )
    {
        lcd_dis( LCD_LINE0, &lcd_line[lcd_curline^1][0] );
        lcd_dis( LCD_LINE1, &lcd_line[lcd_curline][0] );            

        // Do a line feed
        lcd_curline ^= 1;
        lcd_linepos = 0;
        
        for( i = 0; i < LCD_LINE_LENGTH; i++ )
            lcd_line[lcd_curline][i] = ' ';

        return;
    }
    lcd_line[lcd_curline][lcd_linepos++] = c;
}

void
__lcd_outstr(char *s)
{
    char ch;

    while ((ch = *s++) != NULL)
	__lcd_outbyte(ch);
}


/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    init_lcd();

    _bsp_queue_init(&uart1_queue, uart1_buffer, sizeof(uart1_buffer));
    _bsp_queue_init(&uart2_queue, uart2_buffer, sizeof(uart2_buffer));

    init_uart(SIO_BASE_A, 38400);
    init_uart(SIO_BASE_B, 38400);

    __lcd_outstr("Cygnus BSP\n");
}


/*
 * Set any board specific debug traps.
 */
void
_bsp_install_board_debug_traps(void)
{
#ifdef NOT_DONE_YET
    /*
     * None
     */
#endif /* NOT_DONE_YET */
}


static void	cma_ictrl_init(const struct bsp_irq_controller *ic);
static int	cma_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr);
static void	cma_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr);

static bsp_vec_t *cma_irq_vec_tbl[NUM_CMA_INTERRUPTS];

static const struct bsp_irq_controller cma_irq_controller = {
    CMA_IRQ_MIN, CMA_IRQ_MAX,
    &cma_irq_vec_tbl[0],
    cma_ictrl_init,
    cma_ictrl_disable,
    cma_ictrl_enable
};


/*
 * Dispatch code for CMA110 interrupts. Called by exception dispatch code.
 */
static int
cma_irq_dispatch(int ex_nr, ex_regs_t *regs)
{
    bsp_vec_t *vec = NULL;
    int done = 0;
    int irq = 0;
    unsigned char status;

    status = *CMA_ICTL_SRC;

    if (ex_nr == BSP_CORE_EXC_FIQ) {
	if (status & (1 << CMA_IRQ_NMI))
	    irq = CMA_IRQ_NMI;
	else
	    return 0;
    } else {
	for (irq = 0; irq < NUM_CMA_INTERRUPTS; irq++) {
	    if (irq == CMA_IRQ_NMI)
		continue;
	    if (status & (1 << irq))
		break;
	}
    }

    if (irq >= NUM_CMA_INTERRUPTS) {
	/* interrupt source went away */
	bsp_printf("Unexpected IRQ!\n");
	return 1;
    }

    vec = cma_irq_vec_tbl[irq];
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(ex_nr, regs);
	vec = vec->next;
    }

    *CMA_ICTL_CLEAR = (1 << irq);

    return done;
}


/*
 *  Initialize CMA110 interrupt controller.
 */
static void
cma_ictrl_init(const struct bsp_irq_controller *ic)
{
    static bsp_vec_t irq_vec, fiq_vec;
    int i;

    /*
     * Clear all pending interrupts
     */
    *CMA_ICTL_CLEAR = ~0;
    *CMA_ICTL_MASK_WR  = 0;

    for (i = ic->first; i < ic->last; i++)
        ic->vec_list[i - ic->first] = NULL;

    /*
     * Setup exception handlers for the exceptions
     * corresponding to IRQ's and FIQ's
     */
    irq_vec.handler = (void *)cma_irq_dispatch;
    irq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_IRQ, BSP_VEC_CHAIN_FIRST, &irq_vec);

    fiq_vec.handler = (void *)cma_irq_dispatch;
    fiq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_FIQ, BSP_VEC_CHAIN_FIRST, &fiq_vec);
}


/*
 *  Disable CMA110 interrupts.
 */
static int
cma_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned char mask;
    unsigned char bit;

    bit = 1 << irq_nr;
    mask = *CMA_ICTL_MASK_RD;

    *CMA_ICTL_MASK_WR = (mask & ~bit);
    return (mask & bit) != 0;
}


/*
 *  Enable SA110 interrupts.
 */
static void
cma_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned char mask;

    mask = *CMA_ICTL_MASK_RD;
    *CMA_ICTL_MASK_WR = (mask | (1 << irq_nr));
}


/*
 * Install any board specific interrupt controllers.
 */
void
_bsp_install_board_irq_controllers(void)
{
    _bsp_install_irq_controller(&cma_irq_controller);
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
    _bsp_platform_info.cpu = "ARM(R) ARM710T";
    _bsp_platform_info.board = "Cogent CMA222/CMA110";
    _bsp_platform_info.extra = "ARM is a Registered Trademark of Advanced RISC Machines Limited.\nOther Brands and Trademarks are the property of their respective owners.";
}


/*
 * Initialize the mmu and Page Tables
 * The MMU is actually turned on by the caller of this function.
 */
void
_bsp_mmu_init(int ram_size)
{
    unsigned long ttb_base = ((unsigned long)&page1);
    unsigned long i;

    BSP_ASSERT((ttb_base & ARM_TRANSLATION_TABLE_MASK) == ttb_base);

    /*
     * Set the TTB register
     */
    __mcr(ARM_CACHE_COPROCESSOR_NUM,
          ARM_COPROCESSOR_OPCODE_DONT_CARE,
          ttb_base,
          ARM_TRANSLATION_TABLE_BASE_REGISTER,
          ARM_COPROCESSOR_RM_DONT_CARE,
          ARM_COPROCESSOR_OPCODE_DONT_CARE);

    /*
     * Set the Domain Access Control Register
     */
    i = ARM_ACCESS_TYPE_MANAGER(0)    | 
        ARM_ACCESS_TYPE_NO_ACCESS(1)  |
        ARM_ACCESS_TYPE_NO_ACCESS(2)  |
        ARM_ACCESS_TYPE_NO_ACCESS(3)  |
        ARM_ACCESS_TYPE_NO_ACCESS(4)  |
        ARM_ACCESS_TYPE_NO_ACCESS(5)  |
        ARM_ACCESS_TYPE_NO_ACCESS(6)  |
        ARM_ACCESS_TYPE_NO_ACCESS(7)  |
        ARM_ACCESS_TYPE_NO_ACCESS(8)  |
        ARM_ACCESS_TYPE_NO_ACCESS(9)  |
        ARM_ACCESS_TYPE_NO_ACCESS(10) |
        ARM_ACCESS_TYPE_NO_ACCESS(11) |
        ARM_ACCESS_TYPE_NO_ACCESS(12) |
        ARM_ACCESS_TYPE_NO_ACCESS(13) |
        ARM_ACCESS_TYPE_NO_ACCESS(14) |
        ARM_ACCESS_TYPE_NO_ACCESS(15);
    __mcr(ARM_CACHE_COPROCESSOR_NUM,
          ARM_COPROCESSOR_OPCODE_DONT_CARE,
          i,
          ARM_DOMAIN_ACCESS_CONTROL_REGISTER,
          ARM_COPROCESSOR_RM_DONT_CARE,
          ARM_COPROCESSOR_OPCODE_DONT_CARE);

    /*
     * First clear all TT entries - ie Set them to Faulting
     */
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    /*
     * We only do direct mapping for the CMA222/CMA110 board. That is,
     * all virt_addr == phys_addr.
     */

    /*
     * Actual Base = 0x000(00000)
     * Virtual Base = 0x000(00000)
     * Size = Max SDRAM
     * SDRAM
     */
    for (i = 0x000; i < (ram_size >> 20); i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_CACHEABLE, ARM_BUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base  = 0x040(00000) - 0x05E(FFFFF)
     * Virtual Base = 0x040(00000) - 0x05E(FFFFF)
     * Size = 31M
     * PCI Memory Space
     */
    for (i = 0x040; i <= 0x05E; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_UNCACHEABLE, ARM_BUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base = 0x05F(00000)
     * Virtual Base = 0x05F(00000)
     * Size = 1M
     * V360EPC Register Space
     */
    ARM_MMU_SECTION(ttb_base, 0x05F, 0x05F,
		    ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
		    ARM_ACCESS_PERM_RW_RW);

    /*
     * Actual Base  = 0x0E0(00000) - 0x0E7(FFFFF)
     * Virtual Base = 0x0E0(00000) - 0x0E7(FFFFF)
     * Size = 8M
     * ROM
     */
    for (i = 0x0E0; i <= 0x0E7; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_CACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base  = 0x0E8(00000) - 0x0F7(FFFFF)
     * Virtual Base = 0x0E8(00000) - 0x0F7(FFFFF)
     * Size = 16M
     * Motherboard I/O Registers.
     */
    for (i = 0x0E8; i <= 0x0F7; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base  = 0x0F8(00000) - 0x0FF(FFFFF)
     * Virtual Base = 0x0F8(00000) - 0x0FF(FFFFF)
     * Size = 
     * FLASH
     */
    for (i = 0x0F8; i <= 0x0FF; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_CACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }
}

