/*
 * mb86941.h -- Definitions for Fujitsu MB86941 Sparclite Companion Chip
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
#ifndef __MB86941_H__
#define __MB86941_H__ 1

/*
 * Register numbers. These need to be converted into an
 * address offset which is dependent on the particular
 * board implementation.
 */
#define MB86941_TRIG0	0x00
#define MB86941_TRIG1	0x01
#define MB86941_IRQSTAT 0x02
#define MB86941_IRQCLR	0x03
#define MB86941_IRQMASK	0x04
#define MB86941_IRLATCH	0x05
#define MB86941_SERDAT0	0x08
#define MB86941_SERCMD0	0x09
#define MB86941_SERDAT1	0x0C
#define MB86941_SERCMD1	0x0D
#define MB86941_PRS0	0x10
#define MB86941_TCR0	0x11
#define MB86941_RELOAD0	0x12
#define MB86941_CNT0	0x13
#define MB86941_PRS1	0x14
#define MB86941_TCR1	0x15
#define MB86941_RELOAD1	0x16
#define MB86941_CNT1	0x17
#define MB86941_TCR2	0x19
#define MB86941_RELOAD2	0x1A
#define MB86941_CNT2	0x1B
#define MB86941_TCR3	0x1D
#define MB86941_RELOAD3	0x1E
#define MB86941_CNT3	0x1F
#define MB86941_PDAT	0x20
#define MB86941_PDIR	0x21
#define MB86941_SIOCMD  0x24
#define MB86941_SIOSTS  0x25
#define MB86941_SIORX   0x26
#define MB86941_SIOTX   0x27
#define MB86941_SIORATE 0x28
#define MB86941_RTIM0   0x2c
#define MB86941_WTIM0   0x2d
#define MB86941_RTIM1   0x2e
#define MB86941_WTIM1   0x2f


/* Interrupt trigger modes. */
#define TRIG_LEVEL_H   0   /* trigger on high level   */
#define TRIG_LEVEL_L   1   /* trigger on low level    */
#define TRIG_EDGE_H    2   /* trigger on rising edge  */
#define TRIG_EDGE_L    3   /* trigger on falling edge */

/* Timer prescaler register values */
#define PRS_EXTCLK    0x8000
#define PRS_ODIV1     (0<<8)
#define PRS_ODIV2     (1<<8)
#define PRS_ODIV4     (2<<8)
#define PRS_ODIV8     (3<<8)
#define PRS_ODIV16    (4<<8)
#define PRS_ODIV32    (5<<8)
#define PRS_ODIV64    (6<<8)
#define PRS_ODIV128   (7<<8)

/* Timer control register values */
#define TCR_CE		(1<<11)
#define TCR_CLKINT	(0<<9)
#define TCR_CLKEXT	(1<<9)
#define TCR_CLKPRS	(2<<9)
#define TCR_CLKRSVD	(3<<9)
#define TCR_OUTSAME	(0<<7)
#define TCR_OUTHIGH	(1<<7)
#define TCR_OUTLOW	(2<<7)
#define TCR_OUTC3	(3<<7)
#define TCR_INV		(1<<6)
#define TCR_PER_INT	(0<<3)
#define TCR_TO_INT	(1<<3)
#define TCR_SQWAVE	(2<<3)
#define TCR_SW_WATCH	(3<<3)
#define TCR_HW_WATCH	(4<<3)
#define TCR_LEVEL_L	0
#define TCR_LEVEL_H	1
#define TCR_EDGE_H	2
#define TCR_EDGE_L	3
#define TCR_EDGE	4

/* serial mode register values */
#define SER_STOP0	(0<<6)
#define SER_STOP1	(1<<6)
#define SER_STOP1_5	(2<<6)
#define SER_STOP2	(3<<6)
#define SER_NO_PARITY	(0<<4)
#define SER_ODD_PARITY	(1<<4)
#define SER_EVEN_PARITY	(3<<4)
#define SER_5BITS	(0<<2)
#define SER_6BITS	(1<<2)
#define SER_7BITS	(2<<2)
#define SER_8BITS	(3<<2)
#define SER_MODE_SYNCH  0
#define SER_DIV1_CLK    1
#define SER_DIV16_CLK   2
#define SER_DIV64_CLK   3

/* serial command register (asynch) */
#define SER_CMD_EHM	(1<<7)
#define SER_CMD_IRST	(1<<6)
#define SER_CMD_RTS	(1<<5)
#define SER_CMD_EFR	(1<<4)
#define SER_CMD_BREAK	(1<<3)
#define SER_CMD_RXEN	(1<<2)
#define SER_CMD_DTR	(1<<1)
#define SER_CMD_TXEN	(1<<0)

/* serial status register */
#define SER_STAT_DSR    (1<<7)
#define SER_STAT_BREAK  (1<<6)
#define SER_STAT_FERR   (1<<5)
#define SER_STAT_OERR   (1<<4)
#define SER_STAT_PERR   (1<<3)
#define SER_STAT_TXEMP  (1<<2)
#define SER_STAT_RXRDY  (1<<1)
#define SER_STAT_TXRDY  (1<<0)

#if defined(__TOMCAT_EVB__) || defined(__TIGER_EVB__)
#define REGADDR_SHIFT	2
#define REGVAL_SHIFT	0

#define MB_BASE 0x10000000
#define DB_BASE 0x11000000
#endif

static inline unsigned int
mb86941_read(unsigned int base, int regnum)
{
    unsigned int reg_addr;
    unsigned int rval;

    reg_addr = base + (regnum << REGADDR_SHIFT);

    asm volatile("lduh @(%1,gr0), %0\n"
		 "nop\n"
		 "nop\n"
		 "nop\n"
		 : "=r" (rval)
		 : "r" (reg_addr));

    return (rval >> REGVAL_SHIFT);
}


static inline void
mb86941_write(unsigned int base, int regnum, int val)
{
    unsigned int reg_addr;
    unsigned int rval;

    rval = (val << REGVAL_SHIFT);
    reg_addr = base + (regnum << REGADDR_SHIFT);

    asm volatile("sth %0, @(%1,gr0)\n"
		 "nop\n"
		 "nop\n"
		 "nop\n"
		 : /* no outputs */
		 : "r" (rval), "r" (reg_addr) );
}

#endif /* __MB86941_H__ */
