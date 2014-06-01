/*
 * superio.h -- Definitions for FDC37C935 on FR-V eval board.
 *
 * Copyright 1999, 2000 Red Hat, Inc.
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

#define SUPERIO_BASE  ((volatile unsigned *)0x56000000)

#define IO_DELAY() asm volatile ("nop\nnop\nnop\n")

#define IO_WRITE(reg,val) (*(SUPERIO_BASE+(reg)) = ((val)<<16))
#define IO_READ(reg)      (((*(SUPERIO_BASE+(reg)))>>16)&0xff)

/*---------------------------------------------------------------------------
 * PNP configuration defines.
 ---------------------------------------------------------------------------*/

#define CONFIG_PORT 0x370
#define INDEX_PORT  0x370
#define DATA_PORT   0x371

#define CONFIG_START_KEY  0x55
#define CONFIG_END_KEY    0xAA

#define PNP_DEVSEL    0x07   /* device select register index */
#define PNP_ACTIVATE  0x30   /* device activation register index */

#define DEVID_SIO1   4
#define DEVID_SIO2   5

#define BASEADDR_MSB  0x60
#define BASEADDR_LSB  0x61

#define INT_REQ_LEVEL 0x70

#define POWERCTL 0x22
#define SIO_A_PWR 0x10
#define SIO_B_PWR 0x20

#define CFG_WRITE(index,val)    \
{                               \
  IO_WRITE(INDEX_PORT,index);   \
  IO_DELAY();                   \
  IO_WRITE(DATA_PORT,val);      \
  IO_DELAY();                   \
}


/*---------------------------------------------------------------------------
 * 16550 UART definitions.
 ---------------------------------------------------------------------------*/

#define SIO_BASE_A   0x3F8
#define SIO_BASE_B   0x2F8

#define SIO_A_ISAIRQ 5
#define SIO_B_ISAIRQ 4

/* register offsets */
#define SIO_RXD			0	/* receive data, read, dlab = 0 */
#define SIO_TXD			0	/* transmit data, write, dlab = 0 */
#define	SIO_BAUDLO		0	/* baud divisor 0-7, read/write, dlab = 1 */
#define	SIO_IEN			1	/* interrupt enable, read/write, dlab = 0 */
#define	SIO_BAUDHI		1	/* baud divisor 8-15, read/write, dlab = 1 */
#define	SIO_ISTAT		2	/* interrupt status, read, dlab = 0 */
#define	SIO_FCTL		2	/* fifo control, write, dlab = 0 */
#define	SIO_LCTL		3	/* line control, read/write */
#define	SIO_MCTL		4	/* modem control read/write */
#define	SIO_LSTAT		5	/* line status read */
#define	SIO_MSTAT		6	/* modem status read */
#define	SIO_SPR			7	/* scratch pad register */

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
