/*
 * plx9080.h -- Definitions for PLX 9080 PCI Bridge
 *
 * Copyright (c) 1999 Cygnus Support
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
#ifndef __PLX9080_H__
#define __PLX9080_H__ 1

#define PLX_BASE 0x56600000

#define PLX_REG32(X)  *(volatile unsigned *)(PLX_BASE + X)
#define PLX_REG16(X)  *(volatile unsigned short *)(PLX_BASE + X)
#define PLX_REG8(X)   *(volatile unsigned char *)(PLX_BASE + X)

/* PCI Configuration Registers. */
#define PLX_PCIIDR   PLX_REG32(0x00)
#define PLX_PCISR    PLX_REG16(0x04)
#define PLX_PCICR    PLX_REG16(0x06)
#define PLX_PCIBAR0  PLX_REG32(0x10)
#define PLX_PCIBAR1  PLX_REG32(0x14)
#define PLX_PCIBAR2  PLX_REG32(0x18)
#define PLX_PCIBAR3  PLX_REG32(0x1C)
#define PLX_PCIBAR4  PLX_REG32(0x20)
#define PLX_PCIBAR5  PLX_REG32(0x24)
#define PLX_PCICIS   PLX_REG32(0x28)
#define PLX_PCISID   PLX_REG16(0x2C)
#define PLX_PCISVID  PLX_REG16(0x2E)
#define PLX_PCIERBAR PLX_REG16(0x30)
#define PLX_PCIMLR   PLX_REG8(0x3C)
#define PLX_PCIMGR   PLX_REG8(0x3D)
#define PLX_PCIIPR   PLX_REG8(0x3E)
#define PLX_PCIILR   PLX_REG8(0x3F)

/* Local Configuration Registers. */
#define PLX_LAS0RR  PLX_REG32(0x80)
#define PLX_LAS0BA  PLX_REG32(0x84)
#define PLX_MARBR   PLX_REG32(0x88)
#define PLX_BIGEND  PLX_REG32(0x8C)
#define PLX_EROMRR  PLX_REG32(0x90)
#define PLX_EROMBA  PLX_REG32(0x94)
#define PLX_LBRD0   PLX_REG32(0x98)
#define PLX_DMRR    PLX_REG32(0x9C)
#define PLX_DMLBAM  PLX_REG32(0xA0)
#define PLX_DMLBAI  PLX_REG32(0xA4)
#define PLX_DMPBAM  PLX_REG32(0xA8)
#define PLX_DMCFGA  PLX_REG32(0xAC)
#define PLX_LAS1RR  PLX_REG32(0x170)
#define PLX_LAS1BA  PLX_REG32(0x174)
#define PLX_LBRD1   PLX_REG32(0x178)

/* Runtime Registers */
#define PLX_MBOX0    PLX_REG32(0xC0)
#define PLX_MBOX1    PLX_REG32(0xC4)
#define PLX_MBOX2    PLX_REG32(0xC8)
#define PLX_MBOX3    PLX_REG32(0xCC)
#define PLX_MBOX4    PLX_REG32(0xD0)
#define PLX_MBOX5    PLX_REG32(0xD4)
#define PLX_MBOX6    PLX_REG32(0xD8)
#define PLX_MBOX7    PLX_REG32(0xDC)
#define PLX_P2LDBELL PLX_REG32(0xE0)
#define PLX_L2PDBELL PLX_REG32(0xE4)
#define PLX_INTCSR   PLX_REG32(0xE8)
#define PLX_CNTRL    PLX_REG32(0xEC)
#define PLX_PCIHIDR  PLX_REG32(0xF0)

/* DMA Registers */


/* Messaging Queue Registers */


/* PCI Device Config Addresses for DMCFGA register. */
#define PLX_SLOT1_CFGA  0x80000000
#define PLX_SLOT2_CFGA  0x80000800
#define PLX_SLOT3_CFGA  0x80001000
#define PLX_SLOT4_CFGA  0x80001800


#endif  /* __PLX9080_H__ */
