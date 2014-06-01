/*
 * i82596.h -- Header for i82596 Ethernet Controller on Densan R3900 Board
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

#ifndef __I82596_H__
#define __I82596_H__


/*
 * Access to shared data structures is a bit weird on the Densan board.
 * 32-bit accesses are performed as 2 bigendian 16 bit accesses. Those
 * two 16 bit words are stored in little endian order. So, given the
 * address of a 32-bit word:
 *    byte at offset 0 is b8-b15
 *    byte at offset 1 is b0-b7
 *    byte at offset 2 is b24-b31
 *    byte at offset 3 is b16-b23
 */


struct i596 {
    volatile unsigned short port_lsw;
    volatile unsigned short port_msw;
    volatile unsigned char  ca_strobe;	/* any write to strobe CA line */
    char                    pad1[3];
    volatile unsigned char  irq_clear;	/* any write to clear int latch */
    char                    pad2[3];
};

/*
 * Size of tx and rx buffers.
 */
#define I596_BUFSIZE 1532

/*
 * Convert pointer to i586 address space.
 */
static inline unsigned
I596_ADDR(void *addr)
{
    unsigned rval;

    asm (
        "sll  %0,%1,3\n\t"
        "srl  %0,%0,19\n\t"
        "sll  %1,%1,16\n\t"
        "or   %0,%0,%1\n\t"
	: "=r" (rval), "=r" (addr)
	: "1" (addr)
    );

    return rval;
}

/*
 * Convert a cpu pointer to an uncached pointer.
 */
#define CPU_UNCACHED(p) ((void *)((unsigned long)(p) | 0x20000000))

/*
 * Convert an i596 pointer to an uncached cpu pointer.
 */
static inline void *
I596_CPU_UNCACHED(unsigned u)
{
    void *rval;

    asm (
        "ori  %1,%1,0xa000\n\t"
        "sll  %0,%1,16\n\t"
        "srl  %1,%1,16\n\t"
        "or   %0,%0,%1\n\t"
	: "=r" (rval), "=r" (u)
	: "1" (u)
    );

    return rval;
}


#define I596_BASE ((struct i596 *)0xbfbf00c0)

#define I596_RESET() \
{                             \
    I596_BASE->port_lsw = 0;  \
    I596_BASE->port_msw = 0;  \
    __wbflush();              \
}


#define I596_SET_SCP(p)                                       \
{                                                             \
    I596_BASE->port_lsw = ((unsigned short)((long)(p) & 0xfff0)) | 2; \
    I596_BASE->port_msw = (unsigned short)(((long)(p))>>16) & 0x1fff; \
    __wbflush();                                              \
}

#define I596_CHANNEL_ATTN() { I596_BASE->ca_strobe = 0; __wbflush(); }
#define I596_IRQ_CLEAR()    { I596_BASE->irq_clear = 0; __wbflush(); }


/*
 *  System Configuration Pointer
 */
struct scp {
    char  pad1[3];
    char  sysbus;
    int   pad3;
    int   iscp;
};

/*
 * sysbus fields
 */
#define SYSBUS_BE	0x80
#define SYSBUS_INT	0x20
#define SYSBUS_LOCK	0x10
#define SYSBUS_TRG	0x08
#define SYSBUS_82586	0x00
#define SYSBUS_SEGMENT	0x02
#define SYSBUS_LINEAR	0x04


/*
 *  Intermediate System Configuration Pointer
 */
struct iscp {
    char           pad1;
    volatile char  busy;
    short          pad2;
    int            scb;
};


/*
 *  System Control Block
 */
struct scb {
    volatile unsigned short status;
    volatile unsigned short command;
    int                     cba;
    int                     rfa;
    unsigned int            crc;
    unsigned int            align_errs;
    unsigned int            resource_errs;
    unsigned int            overrun_errs;
    unsigned int            rcvcdt;
    unsigned int            short_pkts;
    unsigned short          timer_off;
    unsigned short          timer_on;
};



/*
 *  SCB status macros.
 */
#define SCB_CX		0x8000	/* CU finished a command with its I bit set */
#define SCB_FR		0x4000	/* RU finished receiving a frame */
#define SCB_CNA		0x2000	/* CU not active */
#define SCB_RNR		0x1000	/* RU no ready */
#define SCB_CUS_MASK	0x0700	/* CU state mask */
#define SCB_CUS_IDLE	0x0000	/* CU idle */
#define SCB_CUS_SUSP	0x0100	/* CU suspended */
#define SCB_CUS_ACTIVE	0x0200	/* CU active */
#define SCB_RUS_MASK	0x00f0	/* RU state mask */
#define SCB_RUS_IDLE	0x0000	/* RU idle */
#define SCB_RUS_SUSP	0x0010	/* RU suspended */
#define SCB_RUS_NORES	0x0020	/* RU has no resources */
#define SCB_RUS_RDY	0x0040	/* RU ready */
#define SCB_RUS_RBDRES	0x00a0	/* RU has no resources due to RBDs */
#define SCB_RUS_NORBD	0x00c0	/* RU has no more RBDs */
#define SCB_T		0x0008	/* Bus throttle timers loaded */

/*
 *  SCB command macros.
 */
#define SCB_ACK_CX	0x8000	/* ACK CX status */
#define SCB_ACK_FR	0x4000	/* ACK FR status */
#define SCB_ACK_CNA	0x2000	/* ACK CNA status */
#define SCB_ACK_RNR	0x1000	/* ACK RNR status */
#define SCB_CUC_MASK	0x0700	/* CU command mask */
#define SCB_CUC_NOP	0x0000	/* CU NOP command */
#define SCB_CUC_START	0x0100	/* CU start command execution */
#define SCB_CUC_RESUME	0x0200	/* CU resume command execution */
#define SCB_CUC_SUSPEND	0x0300	/* CU suspend command execution */
#define SCB_CUC_ABORT	0x0400	/* CU abort command execution */
#define SCB_CUC_LOADT	0x0500	/* Load bus throttle timers */
#define SCB_CUC_LOADTR	0x0600	/* Load and restart bus timers */
#define SCB_RESET	0x0080	/* Reset (same as HW reset) */
#define SCB_RUC_MASK	0x0070	/* RU command mask */
#define SCB_RUC_NOP	0x0000	/* RU NOP command */
#define SCB_RUC_START	0x0010	/* RU start receiving */
#define SCB_RUC_RESUME	0x0020	/* RU resume receiving */
#define SCB_RUC_SUSPEND	0x0030	/* RU suspend receiving */
#define SCB_RUC_ABORT	0x0040	/* RU abort receiving */



/*
 *  Simple Command
 */
struct cmd {
    volatile unsigned short status;
    volatile unsigned short command;
    int                     link;
};

/*
 * CMD status macros.
 */
#define CMD_C	0x8000	/* Command complete */
#define CMD_B	0x4000	/* Command busy */
#define CMD_OK	0x2000	/* Command executed ok */


/*
 * CMD command macros.
 */
#define CMD_EL		0x8000	/* End of List */
#define CMD_S		0x4000	/* Suspend CU after execution */
#define CMD_I		0x2000	/* Interrupt after execution */
#define CMD_MASK	0x0007  /* Mask for action commands */
#define CMD_NOP		0x0000  /* NOP command */
#define CMD_IASETUP	0x0001  /* Individual Address setup command */
#define CMD_CONFIGURE	0x0002  /* Configure command */
#define CMD_MCSETUP	0x0003  /* Multicast setup command */
#define CMD_XMIT	0x0004  /* Transmit command */
#define CMD_TDR		0x0005  /* Time Domain Reflectometer command */
#define CMD_DUMP	0x0006  /* Internal info dump command */
#define CMD_DIAG	0x0007  /* Self-test command */
#define CMD_FLEXMODE	0x0008  /* Flex mode for TBDs (CMD_XMIT only) */
#define CMD_NOCRC	0x0010  /* No CRC insertion (CMD_XMIT only) */


struct tbd {
    short   size;
    short   pad;
    int     next;
    int     txbuf;
};

/* indicate last buffer by setting this bit in the TBD size field */
#define TBD_EOF  0x8000


/*
 * tx cmd structure with encapsulated tbd.
 */
struct tx {
    struct cmd cmd;
    int        tbp;
    short      size;
    short      pad;
    struct tbd tbd;
    char       data[I596_BUFSIZE];
};


struct rfd {
    volatile unsigned short status;
    volatile unsigned short command;
    int                     link;
    int                     rbd;
    volatile short          count;
    volatile short          size;
    char                    data[I596_BUFSIZE];
};


struct config_cmd {
    struct cmd	cmd;
    char	data[16];
};

struct ia_cmd {
    struct cmd	cmd;
    char	eth_addr[6];
};

struct tdr_cmd {
    struct cmd	cmd;
    int	        result;
};

#endif  /* __I82596_H__  */
