/*
 * dve39.c -- Support for Densan DVE-R3900 board
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
 */
#include <stdlib.h>
#include <bsp/dve39.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "queue.h"
#include "net.h"
#include "bsplog.h"

static inline void
flash(void)
{
    asm volatile (
      ".set noreorder\n\t"
      "9: lui	$26,0xbfbf\n\t"
      "li	$27,0x40\n\t"
      "sb	$27,3($26)\n\t"
	 "nop\n\t"
	"lui	$8,0x2\n\t"
    "1:	bnez	$8,1b\n\t"
	"addi	$8,$8,-1\n\t"
	"lui	$26,0xbfbf\n\t"
	"li	$27,0x80\n\t"
	"sb	$27,3($26)\n\t"
	" nop\n\t"
	"lui	$8,0x2\n\t"
    "1:	bnez	$8,1b\n\t"
	" addi	$8,$8,-1\n\t"
	"b 9b\n\t"
        " nop\n\t"
      ".set reorder\n\t"
              : : );
}

extern unsigned int __usec_count(void);

static void
__udelay(unsigned n)
{
    unsigned start = __usec_count();
    
    while ((__usec_count() - start) < n)
	;
}

struct schan {
    volatile unsigned char unused0[3];
    volatile unsigned char data;
    volatile unsigned char unused1[3];
    volatile unsigned char control;
};

#define UART_BUFSIZE 16
static bsp_queue_t uart1_queue;
static bsp_queue_t uart2_queue;
static char uart1_buffer[UART_BUFSIZE];
static char uart2_buffer[UART_BUFSIZE];

#define UART_CH_A  ((struct schan *)0xbfbf00d0)
#define UART_CH_B  ((struct schan *)0xbfbf00d8)

#define	R0	0		/* Register selects */
#define	R1	1
#define	R2	2
#define	R3	3
#define	R4	4
#define	R5	5
#define	R6	6
#define	R7	7
#define	R8	8
#define	R9	9
#define	R10	10
#define	R11	11
#define	R12	12
#define	R13	13
#define	R14	14
#define	R15	15

#define	NULLCODE	0	/* Null Code */
#define	POINT_HIGH	0x8	/* Select upper half of registers */
#define	RES_EXT_INT	0x10	/* Reset Ext. Status Interrupts */
#define	RES_CHAN	0x18	/* Channel reset */
#define	RES_RxINT_FC	0x20	/* Reset RxINT on First Character */
#define	RES_Tx_P	0x28	/* Reset TxINT Pending */
#define	ERR_RES		0x30	/* Error Reset */
#define	RES_H_IUS	0x38	/* Reset highest IUS */

#define	RES_Rx_CRC	0x40	/* Reset Rx CRC Checker */
#define	RES_Tx_CRC	0x80	/* Reset Tx CRC Checker */
#define	RES_EOM_L	0xC0	/* Reset EOM latch */

/* Write Register 1 */

#define	EXT_INT_ENAB	0x1	/* Ext Int Enable */
#define	TxINT_ENAB	0x2	/* Tx Int Enable */
#define	PAR_SPEC	0x4	/* Parity is special condition */

#define	RxINT_DISAB	0	/* Rx Int Disable */
#define	RxINT_FCERR	0x8	/* Rx Int on First Character Only or Error */
#define	INT_ALL_Rx	0x10	/* Int on all Rx Characters or error */
#define	INT_ERR_Rx	0x18	/* Int on error only */

#define	WT_RDY_RT	0x20	/* Wait/Ready on R/T */
#define	WT_FN_RDYFN	0x40	/* Wait/FN/Ready FN */
#define	WT_RDY_ENAB	0x80	/* Wait/Ready Enable */

/* Write Register #2 (Interrupt Vector) */

/* Write Register 3 */

#define	RxENABLE	0x1	/* Rx Enable */
#define	SYNC_L_INH	0x2	/* Sync Character Load Inhibit */
#define	ADD_SM		0x4	/* Address Search Mode (SDLC) */
#define	RxCRC_ENAB	0x8	/* Rx CRC Enable */
#define	ENT_HM		0x10	/* Enter Hunt Mode */
#define	AUTO_ENAB	0x20	/* Auto Enables */
#define	Rx5		0x0	/* Rx 5 Bits/Character */
#define	Rx7		0x40	/* Rx 7 Bits/Character */
#define	Rx6		0x80	/* Rx 6 Bits/Character */
#define	Rx8		0xc0	/* Rx 8 Bits/Character */

/* Write Register 4 */

#define	PAR_ENA		0x1	/* Parity Enable */
#define	PAR_EVEN	0x2	/* Parity Even/Odd* */

#define	SYNC_ENAB	0	/* Sync Modes Enable */
#define	SB1		0x4	/* 1 stop bit/char */
#define	SB15		0x8	/* 1.5 stop bits/char */
#define	SB2		0xc	/* 2 stop bits/char */

#define	MONSYNC		0	/* 8 Bit Sync character */
#define	BISYNC		0x10	/* 16 bit sync character */
#define	SDLC		0x20	/* SDLC Mode (01111110 Sync Flag) */
#define	EXTSYNC		0x30	/* External Sync Mode */

#define	X1CLK		0x0	/* x1 clock mode */
#define	X16CLK		0x40	/* x16 clock mode */
#define	X32CLK		0x80	/* x32 clock mode */
#define	X64CLK		0xC0	/* x64 clock mode */

/* Write Register 5 */

#define	TxCRC_ENAB	0x1	/* Tx CRC Enable */
#define	RTS		0x2	/* RTS */
#define	SDLC_CRC	0x4	/* SDLC/CRC-16 */
#define	TxENAB		0x8	/* Tx Enable */
#define	SND_BRK		0x10	/* Send Break */
#define	Tx5		0x0	/* Tx 5 bits (or less)/character */
#define	Tx7		0x20	/* Tx 7 bits/character */
#define	Tx6		0x40	/* Tx 6 bits/character */
#define	Tx8		0x60	/* Tx 8 bits/character */
#define	DTR		0x80	/* DTR */

/* Write Register 6 (Sync bits 0-7/SDLC Address Field) */

/* Write Register 7 (Sync bits 8-15/SDLC 01111110) */

/* Write Register 8 (transmit buffer) */

/* Write Register 9 (Master interrupt control) */
#define	VIS	1	/* Vector Includes Status */
#define	NV	2	/* No Vector */
#define	DLC	4	/* Disable Lower Chain */
#define	MIE	8	/* Master Interrupt Enable */
#define	STATHI	0x10	/* Status high */
#define	NORESET	0	/* No reset on write to R9 */
#define	CHRB	0x40	/* Reset channel B */
#define	CHRA	0x80	/* Reset channel A */
#define	FHWRES	0xc0	/* Force hardware reset */

/* Write Register 10 (misc control bits) */
#define	BITS6	1	/* 6 bit/8bit sync */
#define	LOOPMODE 2	/* SDLC Loop mode */
#define	ABUNDER	4	/* Abort/flag on SDLC xmit underrun */
#define	MARKIDLE 8	/* Mark/flag on idle */
#define	GAOP	0x10	/* Go active on poll */
#define	NRZ	0	/* NRZ mode */
#define	NRZI	0x20	/* NRZI mode */
#define	FM1	0x40	/* FM1 (transition = 1) */
#define	FM0	0x60	/* FM0 (transition = 0) */
#define	CRCPS	0x80	/* CRC Preset I/O */

/* Write Register 12 (lower byte of baud rate generator time constant) */

/* Write Register 13 (upper byte of baud rate generator time constant) */

/* Write Register 14 (Misc control bits) */
#define	BRENABL	1	/* Baud rate generator enable */
#define	BRSRC	2	/* Baud rate generator source */
#define	BSYSCLK	4	/* DTR/Request function */
#define	AUTOECHO 8	/* Auto Echo */
#define	LOOPBAK	0x10	/* Local loopback */
#define	SEARCH	0x20	/* Enter search mode */
#define	RMC	0x40	/* Reset missing clock */
#define	DISDPLL	0x60	/* Disable DPLL */
#define	SSBR	0x80	/* Set DPLL source = BR generator */
#define	SSRTxC	0xa0	/* Set DPLL source = RTxC */
#define	SFMM	0xc0	/* Set FM mode */
#define	SNRZI	0xe0	/* Set NRZI mode */

/* Write Register 15 (Clock Mode control) */
#define	TRxCXT	0	/* TRxC = Xtal output */
#define	TRxCTC	1	/* TRxC = Transmit clock */
#define	TRxCBR	2	/* TRxC = BR Generator Output */
#define	TRxCDP	3	/* TRxC = DPLL output */
#define	TRxCOI	4	/* TRxC O/I */
#define	TCRTxCP	0	/* Transmit clock = RTxC pin */
#define	TCTRxCP	8	/* Transmit clock = TRxC pin */
#define	TCBR	0x10	/* Transmit clock = BR Generator output */
#define	TCDPLL	0x18	/* Transmit clock = DPLL output */
#define	RCRTxCP	0	/* Receive clock = RTxC pin */
#define	RCTRxCP	0x20	/* Receive clock = TRxC pin */
#define	RCBR	0x40	/* Receive clock = BR Generator output */
#define	RCDPLL	0x60	/* Receive clock = DPLL output */
#define	RTxCX	0x80	/* RTxC Xtal/No Xtal */

/* Read Register 0 */
#define	Rx_CH_AV	0x1	/* Rx Character Available */
#define	ZCOUNT		0x2	/* Zero count */
#define	Tx_BUF_EMP	0x4	/* Tx Buffer empty */
#define	DCD		0x8	/* DCD */
#define	SYNC_HUNT	0x10	/* Sync/hunt */
#define	CTS		0x20	/* CTS */
#define	TxEOM		0x40	/* Tx underrun */
#define	BRK_ABRT	0x80	/* Break/Abort */

/* Read Register 1 */
#define	ALL_SNT		0x1	/* All sent */
/* Residue Data for 8 Rx bits/char programmed */
#define	RES3		0x8	/* 0/3 */
#define	RES4		0x4	/* 0/4 */
#define	RES5		0xc	/* 0/5 */
#define	RES6		0x2	/* 0/6 */
#define	RES7		0xa	/* 0/7 */
#define	RES8		0x6	/* 0/8 */
#define	RES18		0xe	/* 1/8 */
#define	RES28		0x0	/* 2/8 */
/* Special Rx Condition Interrupts */
#define	PAR_ERR		0x10	/* Parity error */
#define	Rx_OVR		0x20	/* Rx Overrun Error */
#define	CRC_ERR		0x40	/* CRC/Framing Error */
#define	END_FR		0x80	/* End of Frame (SDLC) */

/* Read Register 2 (channel b only) - Interrupt vector */

/* Read Register 4A (interrupt pending register) ch a only */
#define	CHBEXT	0x1		/* Channel B Ext/Stat IP */
#define	CHBTxIP	0x2		/* Channel B Tx IP */
#define	CHBRxIP	0x4		/* Channel B Rx IP */
#define	CHAEXT	0x8		/* Channel A Ext/Stat IP */
#define	CHATxIP	0x10		/* Channel A Tx IP */
#define	CHARxIP	0x20		/* Channel A Rx IP */

/* Read Register 8 (receive data register) */

/* Read Register 10  (misc status bits) */
#define	ONLOOP	2		/* On loop */
#define	LOOPSEND 0x10		/* Loop sending */
#define	CLK2MIS	0x40		/* Two clocks missing */
#define	CLK1MIS	0x80		/* One clock missing */

/* Read Register 12 (lower byte of baud rate generator constant) */

/* Read Register 13 (upper byte of baud rate generator constant) */

/* Read Register 15 (value of WR 15) */


void
flash_led(int n, int which)
{
    volatile unsigned char *csr0 = (volatile unsigned char *)0xbfbf0003;
    unsigned char val;
    int i;

    if (which)
	which = 0x80;
    else
	which = 0x40;

    val = (*csr0 & ~0xc0);
    *csr0 = val;

    while (n-- > 0) {
	*csr0 = val | which;
	for (i = 0; i < 200000; i++) ;
	*csr0 = val;
	for (i = 0; i < 200000; i++) ;
    }
    *csr0 = val;

    __wbflush();
}

static struct schan *uart_dbg_chan;
static short        ien_a, ien_b;

static void
uart_irq_enable(struct schan *chan)
{
    chan->control = 1;
    __wbflush();
    chan->control = INT_ALL_Rx;
    __wbflush();

    if (chan == UART_CH_A)
	ien_a = 1;
    else
	ien_b = 1;
}

static int
uart_irq_disable(struct schan *chan)
{
    if (chan == UART_CH_A) {
	if (ien_a) {
	    ien_a = 0;
	    chan->control = 1;
	    __wbflush();
	    chan->control = 0;
	    __wbflush();
	    return 1;
	}
    } else if (ien_b) {
	ien_b = 0;
	chan->control = 1;
	__wbflush();
	chan->control = 0;
	__wbflush();
	return 1;
    }
    return 0;
}


static void
uart_putchar(void *chan_data, char ch)
{
    struct schan *chan = chan_data;

    while (!(chan->control & Tx_BUF_EMP))
	;

    chan->data = ch;
    __wbflush();
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
    struct schan *chan = chan_data;

    while (!(chan->control & Rx_CH_AV))
	;

    return chan->data;
}

static int
uart_getchar(void *chan_data)
{
    struct schan *chan = chan_data;
    int ch, was_disabled;

    was_disabled = uart_irq_disable(chan);
    ch = _bsp_dequeue(chan == UART_CH_A ? &uart1_queue : &uart2_queue);
    if (ch < 0)
	ch = uart_rcvchar(chan);

    if (was_disabled)
	uart_irq_enable(chan);
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


static void
uart_irq_clear(struct schan *chan)
{
    volatile unsigned char junk;

    chan->control = 1;      /* read rx errors */
    __wbflush();
    junk = chan->control;
    __udelay(10);
    chan->control = ERR_RES;   /* reset errors */
    __wbflush();
    junk = chan->control;
    __udelay(10);
    UART_CH_A->control = RES_H_IUS; /* reset interrupt */
    __wbflush();
    junk = chan->control;
}


static int
uart_irq_handler(int irq_nr, void *regs)
{
    unsigned char ch, istat;

    /*
     * interrupt status for both channels is
     * gotten from channel A.
     */
    UART_CH_A->control = 4;      /* get interrupt status */
    __wbflush();
    istat = UART_CH_A->control;

    if ((uart_dbg_chan == UART_CH_A && istat & CHARxIP) ||
	(uart_dbg_chan == UART_CH_B && istat & CHBRxIP)) {

	if (uart_dbg_chan->control & Rx_CH_AV) {
	    ch = uart_dbg_chan->data;
	    if (ch == '\003') {
		uart_irq_clear(uart_dbg_chan);
		return 0;
	    }
	    _bsp_enqueue(uart_dbg_chan == UART_CH_A ? \
			 &uart1_queue : \
			 &uart2_queue, ch);
	}
	uart_irq_clear(uart_dbg_chan);

	return 1;
    }
    return 0;
}

#define CLOCK	 9830400
#define CRATEX20 320
#define _brate(bps)		((CLOCK / CRATEX20 / (bps/10)) - 2)

static const int _bauds[] = {
    300, 600, 1200, 2400, 4800, 9600,
    19200, 38400, 0
};

static int uartA_baud, uartB_baud;

static int
uart_setbaud(struct schan *chan, int bps)
{
    int brate, i;

    for (i = 0; _bauds[i]; i++) {
	if (_bauds[i] == bps) {
	    brate = _brate(bps);

	    chan->control = 14;
	    __wbflush();
	    chan->control = 0;
	    __wbflush();
    
	    chan->control = 12;
	    __wbflush();
	    chan->control = 1;
	    __wbflush();
	    chan->control = brate & 0xff;
	    __wbflush();
	    chan->control = (brate >> 8) & 0xff;
	    __wbflush();

	    chan->control = 12;
	    __wbflush();
	    chan->control = 2;
	    __wbflush();
	    chan->control = brate & 0xff;
	    __wbflush();
	    chan->control = (brate >> 8) & 0xff;
	    __wbflush();

	    chan->control = 14;
	    __wbflush();
	    chan->control = BSYSCLK | BRENABL | BRSRC;
	    __wbflush();

	    if (chan == UART_CH_A)
		uartA_baud = bps;
	    else
		uartB_baud = bps;

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
    struct schan *chan = chan_data;
    static bsp_vec_t uart_vec;

    va_start (ap, func);

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = uart_setbaud(chan, i);
	break;

      case COMMCTL_GETBAUD:
	retval = (chan == UART_CH_A) ? uartA_baud : uartB_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	uart_vec.handler = uart_irq_handler;
	uart_vec.next = NULL;

	bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SIO,
			BSP_VEC_REPLACE, &uart_vec);
	bsp_enable_irq(BSP_IRQ_SIO);
	uart_irq_enable(chan);
	uart_dbg_chan = chan;
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	uart_irq_disable(chan);
	bsp_disable_irq(BSP_IRQ_SIO);
	bsp_remove_vec(BSP_VEC_INTERRUPT, BSP_IRQ_SIO, &uart_vec);
	uart_dbg_chan = NULL;
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = uart_irq_disable(chan);
	break;

      case COMMCTL_IRQ_ENABLE:
	uart_irq_enable(chan);
	break;

      default:
	retval = -1;
	break;
    }

    va_end(ap);
    return retval;
}


static void
init_uart(int which, int bps)
{
    struct schan *chan;

    if (which == 0)
	chan = UART_CH_A;
    else
	chan = UART_CH_B;

    chan->control = 0x18;	/* ??? Reset */
    __wbflush();

    /* TX/RX clock source */
    chan->control = 15;
    __wbflush();
    chan->control = RCBR|TCBR|TRxCOI|TRxCBR;
    __wbflush();

    chan->control = 10;
    __wbflush();
    chan->control = 0;
    __wbflush();

    if (which == 0) {
	chan->control = 2;
	__wbflush();
	chan->control = 0;
	__wbflush();
    }

    chan->control = 4;
    __wbflush();
    chan->control = X16CLK | SB1;
    __wbflush();

    chan->control = 3;
    __wbflush();
    chan->control = Rx8 | RxENABLE;
    __wbflush();

    chan->control = 1;
    __wbflush();
    chan->control = 0;
    __wbflush();

    chan->control = 5;
    __wbflush();
    chan->control = DTR | RTS | Tx8 | TxENAB;
    __wbflush();

    uart_setbaud(chan, bps);
}

struct bsp_comm_channel _bsp_comm_list[] = 
{
    {
	{ "uart1", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_CH_A, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    },
    {
	{ "uart2", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)UART_CH_B, uart_write, uart_read,
	  uart_putchar, uart_getchar, uart_control }
    }
};


int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);

#if defined(BSP_LOG)
#define INIT_BAUD 19200
#else
#define INIT_BAUD 38400
#endif

/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    _bsp_queue_init(&uart1_queue, uart1_buffer, sizeof(uart1_buffer));
    _bsp_queue_init(&uart2_queue, uart2_buffer, sizeof(uart2_buffer));

    init_uart(0, INIT_BAUD);
    init_uart(1, INIT_BAUD);

    (void)_bsp_ms_ticks();
}


/*
 * Support routines for DGA-001 interrupt controller module.
 */
static void	dga_ictrl_init(const struct bsp_irq_controller *ic);
static int	dga_ictrl_disable(const struct bsp_irq_controller *ic,
				  int irq_nr);
static void	dga_ictrl_enable(const struct bsp_irq_controller *ic,
				 int irq_nr);

static bsp_vec_t *dga_vec_tbl[BSP_IRQ_ACFAIL - BSP_IRQ_RSVD1 + 1];

static const struct bsp_irq_controller dga_irq_controller = {
    BSP_IRQ_RSVD1, BSP_IRQ_ACFAIL,
    &dga_vec_tbl[0],
    dga_ictrl_init,
    dga_ictrl_disable,
    dga_ictrl_enable
};


static const unsigned char grp0[] = {
    0, BSP_IRQ_SAK,
    BSP_IRQ_SRQ, BSP_IRQ_IAK,
    BSP_IRQ_BUSERR, BSP_IRQ_SYSFAIL,
    BSP_IRQ_ABORT, BSP_IRQ_ACFAIL
};

static const unsigned char grp1[] = { 
    BSP_IRQ_SIO, BSP_IRQ_ETHER,
    BSP_IRQ_SCSI, BSP_IRQ_GP3,
    BSP_IRQ_TT0, BSP_IRQ_TT1,
    BSP_IRQ_DMA, 0
};

static const unsigned char grp2[] = {
    BSP_IRQ_USR0, BSP_IRQ_USR1,
    BSP_IRQ_USR2, BSP_IRQ_USR3,
    BSP_IRQ_USR4, BSP_IRQ_USR5,
    BSP_IRQ_USR6, BSP_IRQ_USR7
};

/*
 * Dispatch code for DGA interrupts.
 */
static int
dga_irq_dispatch(int irq_nr, ex_regs_t *regs)
{
    bsp_vec_t	   *vec;
    unsigned char  dga_vec, grp;
    int		   irq, done;

    /*
     *  Generate interrupt acknowledge.
     */
    switch (irq_nr) {
      case BSP_IRQ_HW0:
	dga_vec = *(volatile unsigned char *)0xbfbd0007;
	break;
      case BSP_IRQ_HW1:
	dga_vec = *(volatile unsigned char *)0xbfbd000b;
	break;
      case BSP_IRQ_HW2:
	dga_vec = *(volatile unsigned char *)0xbfbd000f;
	break;
      case BSP_IRQ_HW3:
	dga_vec = *(volatile unsigned char *)0xbfbd0013;
	break;
      case BSP_IRQ_HW4:
	dga_vec = *(volatile unsigned char *)0xbfbd0017;
	break;
      case BSP_IRQ_HW5:
	dga_vec = *(volatile unsigned char *)0xbfbd001b;
	break;
      case BSP_EXC_NMI:
	dga_vec = *(volatile unsigned char *)0xbfbd001f;
	break;
      default:
	bsp_printf("DGA: Unexpected irq_nr [%d]\n", irq_nr);
	while (1) ;
	break;
    }

    grp = (dga_vec >> 3) & 0x1f;
    
    switch (grp) {
      case 0:
	irq = grp0[dga_vec & 7];
	break;
      case 1:
	irq = grp1[dga_vec & 7];
	break;
      case 2:
	irq = grp2[dga_vec & 7];
	break;
      default:
	/* !!FIXME!! */
	bsp_printf("Unexpected VME IRQ %d:\n", dga_vec);
	while(1);
	break;
    }

    vec = dga_irq_controller.vec_list[irq - dga_irq_controller.first];

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(irq, regs);
	vec = vec->next;
    }

    return done;
}


#define DGA_VECS 7
static bsp_vec_t dga_vecs[DGA_VECS];

/*
 *  Initialize DGA-001 interrupt controller functions.
 */
static void
dga_ictrl_init(const struct bsp_irq_controller *ic)
{
    int i;
    bsp_vec_t *vp;

    for (i = ic->first; i < ic->last; i++)
	ic->vec_list[i - ic->first] = NULL;

    for (vp = &dga_vecs[0]; vp <= &dga_vecs[DGA_VECS - 1]; vp++)
	vp->handler = (bsp_handler_t)dga_irq_dispatch;

    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW0,
		    BSP_VEC_REPLACE, &dga_vecs[0]);
    bsp_enable_irq(BSP_IRQ_HW0);

    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW1,
		    BSP_VEC_REPLACE, &dga_vecs[1]);
    bsp_enable_irq(BSP_IRQ_HW1);

    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW2,
		    BSP_VEC_REPLACE, &dga_vecs[2]);
    bsp_enable_irq(BSP_IRQ_HW2);

    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW3,
		    BSP_VEC_REPLACE, &dga_vecs[3]);
    bsp_enable_irq(BSP_IRQ_HW3);

    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW4,
		    BSP_VEC_REPLACE, &dga_vecs[4]);
    bsp_enable_irq(BSP_IRQ_HW4);

    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_HW5,
		    BSP_VEC_REPLACE, &dga_vecs[5]);
    bsp_enable_irq(BSP_IRQ_HW5);

    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_NMI,
		    BSP_VEC_REPLACE, &dga_vecs[6]);
}


/*
 *  Disable DGA interrupts.
 */
static int
dga_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned u;
    volatile unsigned *csr21 = (volatile unsigned *)CSR21;

    u = *csr21;

    *csr21 = u & ~(1 << irq_nr);

    return ((u & (1 << irq_nr)) != 0);
}


/*
 *  Enable DGA interrupts.
 */
static void
dga_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned u;
    volatile unsigned *csr21 = (volatile unsigned *)CSR21;

    u = *csr21;

    *csr21 = u | (1 << irq_nr);
}


/*
 * Install any board specific interrupt controllers.
 */
void
_bsp_install_board_irq_controllers(void)
{
    _bsp_install_irq_controller(&dga_irq_controller);
}


/*
 * Array of memory region descriptors. We just list RAM and FLASH.
 */
struct bsp_mem_info _bsp_memory_list[] = 
{
    { (void *)0, (void *)0x80000000, 0, 0, BSP_MEM_RAM },
#if defined(BOOT_FROM_FLASH)
    { (void *)0x1fc00000, (void *)0xbfc00000, 0, 4 * 1024 * 1024, BSP_MEM_FLASH }
#else
    { (void *)0x1f000000, (void *)0xbf000000, 0, 4 * 1024 * 1024, BSP_MEM_FLASH }
#endif
};

/*
 * Number of memory region descriptors.
 */
int _bsp_num_mem_regions = 2;


extern void (*_bsp_kill_hook)(void);
extern void _bsp_install_vsr(void);

static void
kill_hook(void)
{
    __dcache_flush(0,-1);
    __icache_flush(0,-1);
    _bsp_install_vsr();
    bsp_debug_irq_enable();
}


#ifdef BSP_LOG
static int
abort_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;

    bsp_log("ABORT: pc[0x%lx] sp[0x%lx]\n", regs->_pc, regs->_sp);
    bsp_log_dump();
    while (1);
    return 0;
}
#endif /* BSP_LOG */

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    extern struct bsp_comm_procs *__enet_link_init(enet_addr_t enet_addr);
    enet_addr_t addr;
    struct bsp_comm_procs *eprocs;
    int      i;
#ifdef BSP_LOG
    static bsp_vec_t abort_vec;
    void *log_start;
#endif

    /*
     * Finish setup of RAM description. Early initialization put a
     * physical pointer to the top of RAM in _bsp_ram_info_ptr.
     */
    _bsp_memory_list[0].nbytes = (long)_bsp_ram_info_ptr;

    /*
     * Override default platform info.
     */
    _bsp_platform_info.cpu = "TX3901AF";
    _bsp_platform_info.board = "Densan DVE R3900/20A";

    /*
     * Override cache info.
     */
    _bsp_dcache_info.size = 1024;
    _bsp_dcache_info.linesize = 4;
    _bsp_dcache_info.ways = 2;

    _bsp_icache_info.size = 4096;
    _bsp_icache_info.linesize = 16;
    _bsp_icache_info.ways = 1;

    kill_hook();

    _bsp_kill_hook = kill_hook;

#ifdef BSP_LOG
    log_start = _bsp_memory_list[0].virt_start + _bsp_memory_list[0].nbytes;
    log_start -= 128 * 1024;

    abort_vec.handler = abort_handler;
    abort_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_ABORT,
                    BSP_VEC_REPLACE, &abort_vec);

    /* put log at top of RAM */
    bsp_log_init((int *)log_start, 128 * 1024, 0);
#endif

    /* enable ABORT switch 'NMI' */
    bsp_enable_irq(BSP_IRQ_ABORT);

    /* Get ethernet address from nvram */
    for (i = 0; i < sizeof(addr); i++)
	addr[i] = NVRAM_ETH_ADDR(i);

#if 0
    /* Get ip address from nvram */
    for (i = 0; i < sizeof(__local_ip_addr); i++)
	__local_ip_addr[i] = NVRAM_IP_ADDR(i);
#endif

    /* Get tcp address from nvram */
    _bsp_net_set_debug_port((NVRAM_TCP_PORT(0) << 8) | NVRAM_TCP_PORT(1));

    eprocs = __enet_link_init(addr);
    if (eprocs) {
	if (!_bsp_net_init(eprocs))
	    bsp_printf("Network initialization failed.\n");
    }
    else
	bsp_printf("Ethernet initialization failed.\n");
}

