/*
 * tiger.c -- Support for Tiger EVB
 *
 * Copyright 2000 Red Hat, Inc.
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
#include "fr500.h"
#include "bsp_if.h"
#include "queue.h"
#include "plx9080.h"
#include "superio.h"
#include "net.h"

#define UART_BUFSIZE 32

static bsp_queue_t sio1_queue;
static char sio1_buffer[UART_BUFSIZE];
static bsp_queue_t sio2_queue;
static char sio2_buffer[UART_BUFSIZE];

#define SIO_IRQ_1  BSP_EXC_INT5
#define SIO_IRQ_2  BSP_EXC_INT4

static int
sio_irq_disable(void *base)
{
    int tty_ien = (int)base + SIO_IEN;
    unsigned char ien;

    ien = IO_READ(tty_ien);
    IO_DELAY();
    IO_WRITE(tty_ien, 0);
    IO_DELAY();

    return (ien & 5) != 0;
}

static void
sio_irq_enable(void *base)
{
    int tty_ien = (int)base + SIO_IEN;

    IO_WRITE(tty_ien, 5);
    IO_DELAY();
}


static void
sio_putchar(void *base, char ch)
{
    int tty_status, tty_tx;

    tty_status = (int)base + SIO_LSTAT;
    tty_tx     = (int)base + SIO_TXD;

    while((IO_READ(tty_status) & SIO_LSTAT_TEMTY) == 0)
	IO_DELAY();

    IO_WRITE(tty_tx, ch);
    IO_DELAY();
}


static void
sio_write(void *base, const char *buf, int len)
{
    while (len-- > 0)
	sio_putchar(base, *buf++);
}


static int
sio_rcvchar(void *base)
{
    int tty_status, tty_rx;
    unsigned char c;

    tty_status = (int)base + SIO_LSTAT;
    tty_rx     = (int)base + SIO_RXD;

    while((IO_READ(tty_status) & SIO_LSTAT_RRDY) == 0)
	IO_DELAY();

    c = IO_READ(tty_rx);
    IO_DELAY();

    return c;
}


static int
sio_getchar(void *base)
{
    int ch, was_enabled;

    was_enabled = sio_irq_disable(base);

    ch = _bsp_dequeue(base == (void*)SIO_BASE_A ? &sio1_queue : &sio2_queue);
    if (ch < 0)
	ch = sio_rcvchar(base);

    if (was_enabled)
	sio_irq_enable(base);
    return ch;
}


static int
sio_read(void *base, char *buf, int len)
{
    int ch;

    if (len <= 0)
	return 0;

    ch = sio_getchar(base);
    *buf = ch;
    return 1;
}


static volatile int sio_dbg_base;

static int
sio_irq_handler(int irq_nr, void *regs)
{
    int tty_istat = sio_dbg_base + SIO_ISTAT;
    int tty_lstat = sio_dbg_base + SIO_LSTAT;
    int ch;
    unsigned char istat, lstat;
    bsp_queue_t *qp;

    istat = IO_READ(tty_istat); /* clear pending irq */
    istat &= 0x0f;
    IO_DELAY();

    qp = (sio_dbg_base == SIO_BASE_A) ? &sio1_queue : &sio2_queue;

    if (istat == 4 || istat == 0x0c) {
	ch = sio_rcvchar((void *)sio_dbg_base);

	if (ch == '\003') {
	    comp_write(COMP_RC, (0x20000 << (irq_nr - BSP_EXC_INT1)));
	    comp_write(COMP_IRL, 0x100000);
	    return 0;
	}

	_bsp_enqueue(qp, ch);

    } else if (istat == 6) {
	lstat = IO_READ(tty_lstat);
    } else if ((istat & 1) == 0)
	bsp_printf("unexpected sio interrupt: istat<0x%x>\n", istat);
    IO_DELAY();

    comp_write(COMP_RC, (0x20000 << (irq_nr - BSP_EXC_INT1)));
    comp_write(COMP_IRL, 0x100000);

    return 1;
}


static int sio1_baud, sio2_baud;

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
sio_setbaud(void *base, int bps)
{
    int   i;
    const struct _baud *bp;
    int lcr = (int)base + SIO_LCTL;
    
    for (i = 0, bp = bauds; i < (sizeof(bauds)/sizeof(bauds[0])); i++, bp++)
	if (bp->baud == bps) {
	    IO_WRITE(lcr, SIO_LCTL_DLATCH);
	    IO_DELAY();

	    IO_WRITE((int)base + SIO_BAUDLO, bp->lsb);
	    IO_DELAY();
	    IO_WRITE((int)base + SIO_BAUDHI, bp->msb);
	    IO_DELAY();

	    IO_WRITE(lcr, SIO_LCTL_8BIT | SIO_LCTL_1STOP | SIO_LCTL_NONE);
	    IO_DELAY();

	    if (base == (void*)SIO_BASE_A)
		sio1_baud = bps;
	    else
		sio2_baud = bps;
	    return 0;
	}
    return -1;
}

static void
init_sio(unsigned char *base, int bps)
{
    sio_setbaud(base, bps);

    /* clear pending irq */
    (void)IO_READ((int)base + SIO_ISTAT);
    IO_DELAY();

    /* clear modem status */
    (void)IO_READ((int)base + SIO_MSTAT);
    IO_DELAY();

    /* enable FIFO */
    IO_WRITE((int)base + SIO_FCTL, 7);
    IO_DELAY();

    /* assert DTR and RTS, enable interrup line */
    IO_WRITE((int)base + SIO_MCTL, 0x0b);
    IO_DELAY();
}


static int
sio_control(void *base, int func, ...)
{
    int          i, retval = 0;
    va_list      ap;
    static bsp_vec_t sio_vec;

    va_start (ap, func);

    i = (base == (void*)SIO_BASE_A) ? SIO_IRQ_1 : SIO_IRQ_2;

    switch (func) {
      case COMMCTL_SETBAUD:
	i = va_arg(ap, int);
	retval = sio_setbaud(base, i);
	break;

      case COMMCTL_GETBAUD:
	retval = (base == (void*)SIO_BASE_A) ? sio1_baud : sio2_baud;
	break;

      case COMMCTL_INSTALL_DBG_ISR:
	sio_vec.handler = sio_irq_handler;
	sio_vec.next = NULL;
	bsp_install_vec(BSP_VEC_EXCEPTION, i,
			BSP_VEC_REPLACE, &sio_vec);
	bsp_enable_irq(i);
	sio_irq_enable(base);
	sio_dbg_base = (int)base;
	break;
	
      case COMMCTL_REMOVE_DBG_ISR:
	sio_irq_disable(base);
	bsp_disable_irq(i);
	bsp_remove_vec(BSP_VEC_EXCEPTION, i, &sio_vec);
	sio_dbg_base = NULL;
	break;

      case COMMCTL_IRQ_DISABLE:
	retval = sio_irq_disable(base);
	break;

      case COMMCTL_IRQ_ENABLE:
	sio_irq_enable(base);
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
	{ (void*)SIO_BASE_A, sio_write, sio_read,
	  sio_putchar, sio_getchar, sio_control }
    },
    {
	{ "uart2", BSP_COMM_SERIAL, BSP_PROTO_NONE },
	{ (void*)SIO_BASE_B, sio_write, sio_read,
	  sio_putchar, sio_getchar, sio_control }
    }
};


int _bsp_num_comms = sizeof(_bsp_comm_list)/sizeof(_bsp_comm_list[0]);

#define DELAY() {volatile int __i = 500000; while (__i-- > 0);}

/*
 * Early initialization of comm channels. Must not rely
 * on interrupts, yet. Interrupt operation can be enabled
 * in _bsp_board_init().
 */
void
_bsp_init_board_comm(void)
{
    IO_WRITE(CONFIG_PORT, CONFIG_START_KEY);
    IO_DELAY();
    IO_WRITE(CONFIG_PORT, CONFIG_START_KEY);
    IO_DELAY();

    CFG_WRITE(PNP_DEVSEL, DEVID_SIO2);
    CFG_WRITE(PNP_ACTIVATE, 0);
    CFG_WRITE(BASEADDR_MSB,0x02);
    CFG_WRITE(BASEADDR_LSB, 0xf8);
    CFG_WRITE(INT_REQ_LEVEL, 4);
    CFG_WRITE(PNP_ACTIVATE, 1);

    CFG_WRITE(PNP_DEVSEL, DEVID_SIO1);
    CFG_WRITE(PNP_ACTIVATE, 0);
    CFG_WRITE(BASEADDR_MSB,0x03);
    CFG_WRITE(BASEADDR_LSB, 0xf8);
    CFG_WRITE(INT_REQ_LEVEL, 5);
    CFG_WRITE(PNP_ACTIVATE, 1);

    CFG_WRITE(POWERCTL,SIO_A_PWR|SIO_B_PWR);

    IO_WRITE(CONFIG_PORT, CONFIG_END_KEY);
    IO_DELAY();

    _bsp_queue_init(&sio1_queue, sio1_buffer, sizeof(sio1_buffer));
    _bsp_queue_init(&sio2_queue, sio2_buffer, sizeof(sio2_buffer));

    init_sio((void*)SIO_BASE_A, 38400);
    init_sio((void*)SIO_BASE_B, 38400);
}


/* Registers used to map ISA IRQs into System Control IRQs */
#define ISA_IRQ0_MAP *((unsigned *)0x56500000)
#define ISA_IRQ1_MAP *((unsigned *)0x56500004)
#define ISA_IRQ2_MAP *((unsigned *)0x56500008)
#define ISA_IRQ3_MAP *((unsigned *)0x5650000c)
#define ISA_IRQ4_MAP *((unsigned *)0x56500010)
#define ISA_IRQ5_MAP *((unsigned *)0x56500014)
#define ISA_IRQ6_MAP *((unsigned *)0x56500018)
#define ISA_IRQ7_MAP *((unsigned *)0x5650001c)

/*
 * Bit fields for ISA_IRQn_MAP registers.
 */
#define ISA_IRQ_ENABLE   (1 << 31)
#define ISA_15           (1 << 30)
#define ISA_14           (1 << 29)
#define ISA_NC13         (1 << 28)
#define ISA_12           (1 << 27)
#define ISA_11           (1 << 26)
#define ISA_10           (1 << 25)
#define ISA_9            (1 << 24)
#define ISA_8            (1 << 23)
#define ISA_7            (1 << 22)
#define ISA_6            (1 << 21)
#define ISA_5            (1 << 20)
#define ISA_4            (1 << 19)
#define ISA_3            (1 << 18)
#define ISA_NC2          (1 << 17)
#define ISA_1            (1 << 16)

/* Registers used to map System Control IRQs into Tiger IRC */
#define SYS_IRQNMI_MAP   *((unsigned *)0x56500030)
#define SYS_IRQ1_MAP     *((unsigned *)0x56500034)
#define SYS_IRQ2_MAP     *((unsigned *)0x56500038)
#define SYS_IRQ3_MAP     *((unsigned *)0x5650003c)
#define SYS_IRQ4_MAP     *((unsigned *)0x56500040)
#define SYS_IRQ5_MAP     *((unsigned *)0x56500044)
#define SYS_IRQ6_MAP     *((unsigned *)0x56500048)
#define SYS_IRQ7_MAP     *((unsigned *)0x5650004c)
#define SYS_IRQ8_MAP     *((unsigned *)0x56500050)
#define SYS_IRQ9_MAP     *((unsigned *)0x56500054)
#define SYS_IRQ10_MAP    *((unsigned *)0x56500058)
#define SYS_IRQ11_MAP    *((unsigned *)0x5650005c)
#define SYS_IRQ12_MAP    *((unsigned *)0x56500060)
#define SYS_IRQ13_MAP    *((unsigned *)0x56500064)
#define SYS_IRQ14_MAP    *((unsigned *)0x56500068)

/*
 * Bit fields for SYS_IRQn_MAP registers.
 */
#define SYS_IRQ_USER1    (1 << 31)
#define SYS_IRQ_PCI      (1 << 30)
#define SYS_IRQ_TRDY1    (1 << 29)
#define SYS_IRQ_TRDY0    (1 << 28)
#define SYS_IRQ_RRDY1    (1 << 27)
#define SYS_IRQ_RRDY0    (1 << 26)
#define SYS_IRQ_TMOUT3   (1 << 25)
#define SYS_IRQ_TMOUT2   (1 << 24)
#define SYS_IRQ_7        (1 << 23)
#define SYS_IRQ_6        (1 << 22)
#define SYS_IRQ_5        (1 << 21)
#define SYS_IRQ_4        (1 << 20)
#define SYS_IRQ_3        (1 << 19)
#define SYS_IRQ_2        (1 << 18)
#define SYS_IRQ_1        (1 << 17)
#define SYS_IRQ_0        (1 << 16)


/*
 * Install any board specific interrupt controllers.
 */
void
_bsp_install_board_irq_controllers(void)
{
    /* init irq remap registers */
    ISA_IRQ0_MAP = ISA_1 | ISA_IRQ_ENABLE;
    ISA_IRQ1_MAP = ISA_8 | ISA_IRQ_ENABLE;
    ISA_IRQ3_MAP = ISA_3 | ISA_IRQ_ENABLE;
    ISA_IRQ4_MAP = ISA_4 | ISA_IRQ_ENABLE;
    ISA_IRQ5_MAP = ISA_5 | ISA_IRQ_ENABLE;
    ISA_IRQ6_MAP = ISA_6 | ISA_IRQ_ENABLE;
    ISA_IRQ7_MAP = ISA_7 | ISA_IRQ_ENABLE;

    SYS_IRQ1_MAP   = SYS_IRQ_TMOUT3;
    SYS_IRQ2_MAP   = SYS_IRQ_PCI;
    SYS_IRQ3_MAP   = SYS_IRQ_3;
    SYS_IRQ4_MAP   = SYS_IRQ_4;      /* COM2 */
    SYS_IRQ5_MAP   = SYS_IRQ_5;      /* COM1 */
    SYS_IRQ6_MAP   = SYS_IRQ_TRDY1;
    SYS_IRQ7_MAP   = SYS_IRQ_RRDY1;
    SYS_IRQ8_MAP   = SYS_IRQ_TMOUT2;
    SYS_IRQ9_MAP   = SYS_IRQ_TRDY0;
    SYS_IRQ10_MAP  = SYS_IRQ_RRDY0;
}


/*
 * Array of memory region descriptors.
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
extern void _bsp_init_ms_timer(void);

static void
kill_hook(void)
{
    extern void *bsp_vsr_table[];
    extern void _bsp_clock_vsr(void);
    extern void _bsp_init_clock_timer(void);

    _bsp_install_vsr();

    /* mask individual interrupts, unmask IRL */
    comp_write(COMP_MASK, 0xfffe0000);

    /* setup triggers for interrupts 15-8 */
    comp_write(COMP_TM0,
	       (TRIG_LEVEL_L << 30) | (TRIG_LEVEL_L << 28) |
	       (TRIG_LEVEL_H << 26) | (TRIG_LEVEL_L << 14) |
	       (TRIG_LEVEL_L << 22) | (TRIG_LEVEL_H << 20) |
	       (TRIG_LEVEL_H << 18) | (TRIG_LEVEL_H << 16));

    /* setup triggers for interrupts 7-1 */
    comp_write(COMP_TM1,
	       (TRIG_LEVEL_H << 30) | (TRIG_LEVEL_H << 28) |
	       (TRIG_EDGE_H  << 26) | (TRIG_EDGE_H << 24) |
	       (TRIG_EDGE_H << 22) );

    /* clear any sensed interrupts */
    comp_write(COMP_RC, 0xfffe0000);

    /* clear IRL */
    comp_write(COMP_IRL, 0x100000);

    _bsp_init_ms_timer();

    /* setup clock */
    bsp_vsr_table[BSP_EXC_INT8] = _bsp_clock_vsr;
    _bsp_init_clock_timer();
    bsp_enable_irq(BSP_EXC_INT8);

    bsp_debug_irq_enable();

#if 0
    bsp_enable_irq(BSP_EXC_INT15); /* NMI */
#endif
}


void
init_pci(void)
{
    volatile unsigned dummy;

    /* Map Local->PCI memory */
    PLX_DMRR = 0xfff00000;    /* 1MB */
    PLX_DMLBAI = 0xf6800000;  /* i/o base (actually 0x56800000 to cpu) */
    PLX_DMPBAM = 0x00000002;  /* I/O remap select */

#if 0
    /* Map PCI->SDRAM space */
    PLX_LAS0RR = 0xfe000000;  /* 32MB */
    PLX_LAS0BA = 1;           /* enable */
#endif

    PLX_PCIBAR0 = 0xffffffff;
    dummy = PLX_PCIBAR0;
    PLX_PCIBAR0 = 0x80000000;

    PLX_PCIBAR1 = 0xffffffff;
    dummy = PLX_PCIBAR1;
    PLX_PCIBAR1 = 0x90000001;

    PLX_PCIBAR2 = 0xffffffff;
    dummy = PLX_PCIBAR2;
    PLX_PCIBAR2 = 0;

    PLX_PCICR   = 5;  /* Master Enable + I/O Enable */

    PLX_CNTRL = 0x8000767e;  /* init done */
}

extern struct bsp_comm_procs *__enet_link_init(void);
extern int __enet_pci_probe(void);

#define ICACHE_SIZE    (16*1024)
#define ICACHE_LSIZE   64
#define ICACHE_WAYS    4
#define ICACHE_INDICES (ICACHE_SIZE / (ICACHE_LSIZE*ICACHE_WAYS))

#define DCACHE_SIZE    (16*1024)
#define DCACHE_LSIZE   64
#define DCACHE_WAYS    4
#define DCACHE_INDICES (DCACHE_SIZE / (DCACHE_LSIZE*DCACHE_WAYS))

/*
 *  Board specific BSP initialization.
 */
void
_bsp_board_init(void)
{
    struct bsp_comm_procs *eprocs;

    /*
     * Finish setup of RAM description. Early initialization put a
     * physical pointer to the top of RAM in _bsp_ram_info_ptr.
     */
    _bsp_memory_list[0].nbytes = (long)_bsp_ram_info_ptr;

    _bsp_dcache_info.size = DCACHE_SIZE;
    _bsp_dcache_info.linesize = DCACHE_LSIZE;
    _bsp_dcache_info.ways = DCACHE_WAYS;

    _bsp_icache_info.size = ICACHE_SIZE;
    _bsp_icache_info.linesize = ICACHE_LSIZE;
    _bsp_icache_info.ways = ICACHE_WAYS;

    /*
     * Override default platform info.
     */
    _bsp_platform_info.cpu = "FR-V";
    _bsp_platform_info.board = "FR-V EVB";

    kill_hook();

    _bsp_kill_hook = kill_hook;

    init_pci();

    if (__enet_pci_probe()) {
	eprocs = __enet_link_init();
	if (eprocs) {
	    if (!_bsp_net_init(eprocs))
		bsp_printf("Network initialization failed.\n");
	}
	else
	    bsp_printf("Ethernet initialization failed.\n");
    }
}


/*
 * Flush Icache for given range. If nbytes < 0, flush entire cache.
 */
void
__icache_flush(void *addr, int nbytes)
{
    int lines;
    unsigned long ul = (unsigned long)addr;

    asm volatile ("bar\n");

    if (nbytes == 0)
	return;

    if (nbytes > 0) {
	/* flush just the range. */
	nbytes += (ul & (ICACHE_LSIZE-1));
	ul     &= ~(ICACHE_LSIZE-1);
	lines  = (nbytes + ICACHE_LSIZE - 1) / ICACHE_LSIZE;
	if (lines > ICACHE_INDICES)
	    lines = ICACHE_INDICES;

	while (lines-- > 0) {
	    asm volatile ("\tici	@(%0,gr0)\n"
			  : /* no outputs */
			  : "r" (ul) );
	    ul += ICACHE_LSIZE;
	}
	return;
    }
#if 0
    /* flush entire cache */
    /* not supported */
#endif
}


/*
 * Flush Dcache for given range. If nbytes < 0, flush entire cache.
 */
void
__dcache_flush(void *addr, int nbytes)
{
    int lines;
    unsigned long ul = (unsigned long)addr;

    asm volatile ("membar\n");

    if (nbytes == 0)
	return;

    if (nbytes > 0) {
	/* flush just the range. */
	nbytes += (ul & (DCACHE_LSIZE-1));
	ul     &= ~(DCACHE_LSIZE-1);
	lines  = (nbytes + DCACHE_LSIZE - 1) / DCACHE_LSIZE;
	if (lines > DCACHE_INDICES)
	    lines = DCACHE_INDICES;

	while (lines-- > 0) {
	    asm volatile ("\tdcf	@(%0,gr0)\n"
			  : /* no outputs */
			  : "r" (ul) );
	    ul += DCACHE_LSIZE;
	}
	return;
    }
#if 0
    /* flush entire cache */
    /* not supported */
#endif
    asm volatile ("membar\n");
}

