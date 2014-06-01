/*
 * mb86964.h -- Definitions for Fujitsu MB86964 NIC on MB91V101 eval board.
 *
 * Copyright (c) 1998 Cygnus Support
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
#ifndef __MB86964_H__
#define __MB86964_H__

typedef volatile unsigned char  reg8;
typedef volatile unsigned short reg16;
typedef volatile unsigned int   reg32;

/*
 *  MB86964 layout for Cogent CMA101 Motherboard.
 */
struct mb86964_regs {
    reg16    status;	/* Transmit/Receive Status */
#define TX_DONE     0x8000
#define NET_BSY     0x4000
#define TX_PKT_RCD  0x2000
#define CR_LOST     0x1000
#define JABBER      0x0800
#define COLLISION   0x0400
#define COLLISIONS  0x0200
#define RX_PKT      0x0080
#define BUS_RD_ERR  0x0040
#define DMA_EOP     0x0020
#define RMT_CNTRL   0x0010
#define SHORT_PKT   0x0008
#define ALIGN_ERR   0x0004
#define CRC_ERR     0x0002
#define BUF_OVRFLO  0x0001

    reg16   ien;	/* Transmit/Receive Interrupt Enable */

    reg16   mode;	/* Transmit/Receive Mode */
#define RX_BUF_EMPTY      0x40
#define ACCEPT_BAD_PKTS   0x20
#define RX_SHORT_ADDR     0x10
#define ACCEPT_SHORT_PKTS 0x08
#define RMT_CNTRL_EN      0x04

    reg16   config;    /* Configuration 0/1 */

    union {
	struct {
	    reg16   node_01;
	    reg16   node_23;
	    reg16   node_45;
	    reg16   tdr;
	} rbs0;
	struct {
	    reg16   hash_01;
	    reg16   hash_23;
	    reg16   hash_45;
	    reg16   hash_67;
	} rbs1;
	struct {
	    reg16   buffer;
	    reg16   tx_start;	/* tx start, packet count, collision */
	    reg16   dma;
	    reg16   xcvr;
	} rbs2;
    } u;
};


struct mb86964_regs *mb_regs = ((struct mb86964_regs *)0x00300000);

#endif /* __MB86964_H__ */
