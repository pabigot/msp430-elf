/*
 * dve39.h -- Header for Densan R3900 VME board.
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
#ifndef __DVE39_H__
#define __DVE39_H__ 1

#define BSP_IRQ_RSVD1  9
#define BSP_IRQ_VME1  10
#define BSP_IRQ_VME2  11
#define BSP_IRQ_VME3  12
#define BSP_IRQ_VME4  13
#define BSP_IRQ_VME5  14
#define BSP_IRQ_VME6  15
#define BSP_IRQ_VME7  16
#define BSP_IRQ_USR0  17
#define BSP_IRQ_USR1  18
#define BSP_IRQ_USR2  19
#define BSP_IRQ_USR3  20
#define BSP_IRQ_USR4  21
#define BSP_IRQ_USR5  22
#define BSP_IRQ_USR6  23
#define BSP_IRQ_USR7  24
#define BSP_IRQ_SIO   25
#define BSP_IRQ_ETHER 26
#define BSP_IRQ_SCSI  27
#define BSP_IRQ_GP3   28
#define BSP_IRQ_TT0   29
#define BSP_IRQ_TT1   30
#define BSP_IRQ_DMA   31
#define BSP_IRQ_GP7   32
#define BSP_IRQ_RSVD2 33
#define BSP_IRQ_SAK   34
#define BSP_IRQ_SRQ   35
#define BSP_IRQ_IAK   36
#define BSP_IRQ_BUSERR  37
#define BSP_IRQ_SYSFAIL 38
#define BSP_IRQ_ABORT   39
#define BSP_IRQ_ACFAIL  40


#define CSR0	0xbfbf0000
#define CSR1	0xbfbf0004
#define CSR2	0xbfbf0008
#define CSR3	0xbfbf000c
#define CSR4	0xbfbf0010
#define CSR5	0xbfbf0014
#define CSR6	0xbfbf0018
#define CSR7	0xbfbf001c
#define CSR8	0xbfbf0020
#define CSR9	0xbfbf0024
#define CSR10	0xbfbf0028
#define CSR11	0xbfbf002c
#define CSR12	0xbfbf0030
#define CSR13	0xbfbf0034
#define CSR14	0xbfbf0038
#define CSR15	0xbfbf003c
#define CSR16	0xbfbf0040
#define CSR17	0xbfbf0044
#define CSR18	0xbfbf0048
#define CSR19	0xbfbf004c
#define CSR20	0xbfbf0050
#define CSR21	0xbfbf0054
#define CSR22	0xbfbf0058
#define CSR23	0xbfbf005c
#define CSR24	0xbfbf0060
#define CSR25	0xbfbf0064
#define CSR26	0xbfbf0068
#define CSR27	0xbfbf006c
#define IFR0	0xbfbf0070
#define IFR1	0xbfbf0074
#define IFR2	0xbfbf0078
#define IFR3	0xbfbf007c

/*
 * CSR0 bit descriptions.
 */
#define CSR0_LB0	0x00000000
#define CSR0_LB1	0x10000000
#define CSR0_LB2	0x20000000
#define CSR0_LB3	0x30000000
#define CSR0_TAOUTE	0x08000000
#define CSR0_EBIG	0x04000000
#define CSR0_LBM_MASK	0x03000000
#define CSR0_BERDYO	0x00800000
#define CSR0_LBTOUT	0x00040000
#define CSR0_RQTOUT	0x00020000
#define CSR0_VMBERR	0x00010000
#define CSR0_GPIOE7	0x00008000
#define CSR0_GPIOE6	0x00004000
#define CSR0_GPIOE5	0x00002000
#define CSR0_GPIOE4	0x00001000
#define CSR0_GPIOE3	0x00000800
#define CSR0_GPIOE2	0x00000400
#define CSR0_GPIOE1	0x00000200
#define CSR0_GPIOE0	0x00000100
#define CSR0_GPIOE7	0x00008000
#define CSR0_GPIO7	0x00000080
#define CSR0_GPIO6	0x00000040
#define CSR0_GPIO5	0x00000020
#define CSR0_GPIO4	0x00000010
#define CSR0_GPIO3	0x00000008
#define CSR0_GPIO2	0x00000004
#define CSR0_GPIO1	0x00000002
#define CSR0_GPIO0	0x00000001

/*
 * CSR1 bit descriptions.
 */
#define CSR1_PURSTF	0x80000000
#define CSR1_RSWEN	0x40000000
#define CSR1_RSWSYS	0x20000000
#define CSR1_SYSCON	0x00800000
#define CSR1_RRS	0x00100000
#define CSR1_ARB0	0x00000000
#define CSR1_ARB1	0x00040000
#define CSR1_ARB2	0x00080000
#define CSR1_ARB3	0x000c0000
#define CSR1_SBT0	0x00000000
#define CSR1_SBT1	0x00010000
#define CSR1_SBT2	0x00020000
#define CSR1_SBT3	0x00030000
#define CSR1_DWB	0x00008000
#define CSR1_OWN	0x00004000
#define CSR1_RQT0	0x00000000
#define CSR1_RQT1	0x00001000
#define CSR1_RQT2	0x00002000
#define CSR1_RQT3	0x00003000
#define CSR1_RELMD0	0x00000000
#define CSR1_RELMD1	0x00000400
#define CSR1_RELMD2	0x00000800
#define CSR1_RELMD3	0x00000c00
#define CSR1_RQLEV0	0x00000000
#define CSR1_RQLEV1	0x00000100
#define CSR1_RQLEV2	0x00000200
#define CSR1_RQLEV3	0x00000300
#define CSR1_RMC	0x00000080
#define CSR1_AAMEN	0x00000040

/*
 * CSR3 bit descriptions.
 */
#define CSR3_IFRSUP	0x00000002
#define CSR3_IFRUSR	0x00000001

/*
 * CSR20 bit descriptions.
 */
#define CSR20_ACFIRQ    (1 << 31)
#define CSR20_ABTIRQ    (1 << 30)
#define CSR20_SFIRQ     (1 << 29)
#define CSR20_BERIRQ    (1 << 28)
#define CSR20_IAKIRQ    (1 << 27)
#define CSR20_SRQIRQ    (1 << 26)
#define CSR20_SAKIRQ    (1 << 25)
#define CSR20_UNUSED24  (1 << 24)
#define CSR20_GP7IRQ    (1 << 23)
#define CSR20_DMAIRQ    (1 << 22)
#define CSR20_TT1IRQ    (1 << 21)
#define CSR20_TT0IRQ    (1 << 20)
#define CSR20_GP3IRQ    (1 << 19)
#define CSR20_GP2IRQ    (1 << 18)
#define CSR20_GP1IRQ    (1 << 17)
#define CSR20_GP0IRQ    (1 << 16)
#define CSR20_SW7IRQ    (1 << 15)
#define CSR20_SW6IRQ    (1 << 14)
#define CSR20_SW5IRQ    (1 << 13)
#define CSR20_SW4IRQ    (1 << 12)
#define CSR20_SW3IRQ    (1 << 11)
#define CSR20_SW2IRQ    (1 << 10)
#define CSR20_SW1IRQ    (1 <<  9)
#define CSR20_SW0IRQ    (1 <<  8)
#define CSR20_VM7IRQ    (1 <<  7)
#define CSR20_VM6IRQ    (1 <<  6)
#define CSR20_VM5IRQ    (1 <<  5)
#define CSR20_VM4IRQ    (1 <<  4)
#define CSR20_VM3IRQ    (1 <<  3)
#define CSR20_VM2IRQ    (1 <<  2)
#define CSR20_VM1IRQ    (1 <<  1)
#define CSR20_UNUSED0   (1 <<  0)


/*
 * CSR24 bit descriptions.
 */
#define CSR24_ACFNMI	0x80000000
#define CSR24_ACFIL1	(1 << 28)
#define CSR24_ACFIL2	(2 << 28)
#define CSR24_ACFIL3	(3 << 28)
#define CSR24_ACFIL4	(4 << 28)
#define CSR24_ACFIL5	(5 << 28)
#define CSR24_ACFIL6	(6 << 28)
#define CSR24_ACFIL7	(7 << 28)
#define CSR24_ABTNMI	0x08000000
#define CSR24_ABTIL1	(1 << 24)
#define CSR24_ABTIL2	(2 << 24)
#define CSR24_ABTIL3	(3 << 24)
#define CSR24_ABTIL4	(4 << 24)
#define CSR24_ABTIL5	(5 << 24)
#define CSR24_ABTIL6	(6 << 24)
#define CSR24_ABTIL7	(7 << 24)
#define CSR24_SFNMI	0x00800000
#define CSR24_SFIL1	(1 << 20)
#define CSR24_SFIL2	(2 << 20)
#define CSR24_SFIL3	(3 << 20)
#define CSR24_SFIL4	(4 << 20)
#define CSR24_SFIL5	(5 << 20)
#define CSR24_SFIL6	(6 << 20)
#define CSR24_SFIL7	(7 << 20)
#define CSR24_BERNMI	0x00080000
#define CSR24_BERIL1	(1 << 16)
#define CSR24_BERIL2	(2 << 16)
#define CSR24_BERIL3	(3 << 16)
#define CSR24_BERIL4	(4 << 16)
#define CSR24_BERIL5	(5 << 16)
#define CSR24_BERIL6	(6 << 16)
#define CSR24_BERIL7	(7 << 16)
#define CSR24_IAKNMI	0x00008000
#define CSR24_IAKIL1	(1 << 12)
#define CSR24_IAKIL2	(2 << 12)
#define CSR24_IAKIL3	(3 << 12)
#define CSR24_IAKIL4	(4 << 12)
#define CSR24_IAKIL5	(5 << 12)
#define CSR24_IAKIL6	(6 << 12)
#define CSR24_IAKIL7	(7 << 12)
#define CSR24_SRQNMI	0x00000800
#define CSR24_SRQIL1	(1 << 8)
#define CSR24_SRQIL2	(2 << 8)
#define CSR24_SRQIL3	(3 << 8)
#define CSR24_SRQIL4	(4 << 8)
#define CSR24_SRQIL5	(5 << 8)
#define CSR24_SRQIL6	(6 << 8)
#define CSR24_SRQIL7	(7 << 8)
#define CSR24_SAKNMI	0x00000080
#define CSR24_SAKIL1	(1 << 4)
#define CSR24_SAKIL2	(2 << 4)
#define CSR24_SAKIL3	(3 << 4)
#define CSR24_SAKIL4	(4 << 4)
#define CSR24_SAKIL5	(5 << 4)
#define CSR24_SAKIL6	(6 << 4)
#define CSR24_SAKIL7	(7 << 4)


/*
 * CSR25 bit descriptions.
 */
#define CSR25_DMANMI	0x08000000
#define CSR25_DMAIL1	(1 << 24)
#define CSR25_DMAIL2	(2 << 24)
#define CSR25_DMAIL3	(3 << 24)
#define CSR25_DMAIL4	(4 << 24)
#define CSR25_DMAIL5	(5 << 24)
#define CSR25_DMAIL6	(6 << 24)
#define CSR25_DMAIL7	(7 << 24)
#define CSR25_TT1NMI	0x00800000
#define CSR25_TT1IL1	(1 << 20)
#define CSR25_TT1IL2	(2 << 20)
#define CSR25_TT1IL3	(3 << 20)
#define CSR25_TT1IL4	(4 << 20)
#define CSR25_TT1IL5	(5 << 20)
#define CSR25_TT1IL6	(6 << 20)
#define CSR25_TT1IL7	(7 << 20)
#define CSR25_TT0NMI	0x00080000
#define CSR25_TT0IL1	(1 << 16)
#define CSR25_TT0IL2	(2 << 16)
#define CSR25_TT0IL3	(3 << 16)
#define CSR25_TT0IL4	(4 << 16)
#define CSR25_TT0IL5	(5 << 16)
#define CSR25_TT0IL6	(6 << 16)
#define CSR25_TT0IL7	(7 << 16)
#define CSR25_GP3NMI	0x00008000
#define CSR25_GP3IL1	(1 << 12)
#define CSR25_GP3IL2	(2 << 12)
#define CSR25_GP3IL3	(3 << 12)
#define CSR25_GP3IL4	(4 << 12)
#define CSR25_GP3IL5	(5 << 12)
#define CSR25_GP3IL6	(6 << 12)
#define CSR25_GP3IL7	(7 << 12)
#define CSR25_GP2NMI	0x00000800
#define CSR25_GP2IL1	(1 << 8)
#define CSR25_GP2IL2	(2 << 8)
#define CSR25_GP2IL3	(3 << 8)
#define CSR25_GP2IL4	(4 << 8)
#define CSR25_GP2IL5	(5 << 8)
#define CSR25_GP2IL6	(6 << 8)
#define CSR25_GP2IL7	(7 << 8)
#define CSR25_GP1NMI	0x00000080
#define CSR25_GP1IL1	(1 << 4)
#define CSR25_GP1IL2	(2 << 4)
#define CSR25_GP1IL3	(3 << 4)
#define CSR25_GP1IL4	(4 << 4)
#define CSR25_GP1IL5	(5 << 4)
#define CSR25_GP1IL6	(6 << 4)
#define CSR25_GP1IL7	(7 << 4)
#define CSR25_GP0NMI	0x00000008
#define CSR25_GP0IL1	(1 << 0)
#define CSR25_GP0IL2	(2 << 0)
#define CSR25_GP0IL3	(3 << 0)
#define CSR25_GP0IL4	(4 << 0)
#define CSR25_GP0IL5	(5 << 0)
#define CSR25_GP0IL6	(6 << 0)
#define CSR25_GP0IL7	(7 << 0)

/*
 * CSR26 bit descriptions.
 */
#define CSR26_SW7NMI	0x08000000
#define CSR26_SW7IL1	(1 << 28)
#define CSR26_SW7IL2	(2 << 28)
#define CSR26_SW7IL3	(3 << 28)
#define CSR26_SW7IL4	(4 << 28)
#define CSR26_SW7IL5	(5 << 28)
#define CSR26_SW7IL6	(6 << 28)
#define CSR26_SW7IL7	(7 << 28)
#define CSR26_SW6NMI	0x08000000
#define CSR26_SW6IL1	(1 << 24)
#define CSR26_SW6IL2	(2 << 24)
#define CSR26_SW6IL3	(3 << 24)
#define CSR26_SW6IL4	(4 << 24)
#define CSR26_SW6IL5	(5 << 24)
#define CSR26_SW6IL6	(6 << 24)
#define CSR26_SW6IL7	(7 << 24)
#define CSR26_SW5NMI	0x00800000
#define CSR26_SW5IL1	(1 << 20)
#define CSR26_SW5IL2	(2 << 20)
#define CSR26_SW5IL3	(3 << 20)
#define CSR26_SW5IL4	(4 << 20)
#define CSR26_SW5IL5	(5 << 20)
#define CSR26_SW5IL6	(6 << 20)
#define CSR26_SW5IL7	(7 << 20)
#define CSR26_SW4NMI	0x00080000
#define CSR26_SW4IL1	(1 << 16)
#define CSR26_SW4IL2	(2 << 16)
#define CSR26_SW4IL3	(3 << 16)
#define CSR26_SW4IL4	(4 << 16)
#define CSR26_SW4IL5	(5 << 16)
#define CSR26_SW4IL6	(6 << 16)
#define CSR26_SW4IL7	(7 << 16)
#define CSR26_SW3NMI	0x00008000
#define CSR26_SW3IL1	(1 << 12)
#define CSR26_SW3IL2	(2 << 12)
#define CSR26_SW3IL3	(3 << 12)
#define CSR26_SW3IL4	(4 << 12)
#define CSR26_SW3IL5	(5 << 12)
#define CSR26_SW3IL6	(6 << 12)
#define CSR26_SW3IL7	(7 << 12)
#define CSR26_SW2NMI	0x00000800
#define CSR26_SW2IL1	(1 << 8)
#define CSR26_SW2IL2	(2 << 8)
#define CSR26_SW2IL3	(3 << 8)
#define CSR26_SW2IL4	(4 << 8)
#define CSR26_SW2IL5	(5 << 8)
#define CSR26_SW2IL6	(6 << 8)
#define CSR26_SW2IL7	(7 << 8)
#define CSR26_SW1NMI	0x00000080
#define CSR26_SW1IL1	(1 << 4)
#define CSR26_SW1IL2	(2 << 4)
#define CSR26_SW1IL3	(3 << 4)
#define CSR26_SW1IL4	(4 << 4)
#define CSR26_SW1IL5	(5 << 4)
#define CSR26_SW1IL6	(6 << 4)
#define CSR26_SW1IL7	(7 << 4)
#define CSR26_SW0NMI	0x00000008
#define CSR26_SW0IL1	(1 << 0)
#define CSR26_SW0IL2	(2 << 0)
#define CSR26_SW0IL3	(3 << 0)
#define CSR26_SW0IL4	(4 << 0)
#define CSR26_SW0IL5	(5 << 0)
#define CSR26_SW0IL6	(6 << 0)
#define CSR26_SW0IL7	(7 << 0)


/*
 * CSR27 bit descriptions.
 */
#define CSR27_VM7IL1	(1 << 28)
#define CSR27_VM7IL2	(2 << 28)
#define CSR27_VM7IL3	(3 << 28)
#define CSR27_VM7IL4	(4 << 28)
#define CSR27_VM7IL5	(5 << 28)
#define CSR27_VM7IL6	(6 << 28)
#define CSR27_VM7IL7	(7 << 28)
#define CSR27_VM6IL1	(1 << 24)
#define CSR27_VM6IL2	(2 << 24)
#define CSR27_VM6IL3	(3 << 24)
#define CSR27_VM6IL4	(4 << 24)
#define CSR27_VM6IL5	(5 << 24)
#define CSR27_VM6IL6	(6 << 24)
#define CSR27_VM6IL7	(7 << 24)
#define CSR27_VM5IL1	(1 << 20)
#define CSR27_VM5IL2	(2 << 20)
#define CSR27_VM5IL3	(3 << 20)
#define CSR27_VM5IL4	(4 << 20)
#define CSR27_VM5IL5	(5 << 20)
#define CSR27_VM5IL6	(6 << 20)
#define CSR27_VM5IL7	(7 << 20)
#define CSR27_VM4IL1	(1 << 16)
#define CSR27_VM4IL2	(2 << 16)
#define CSR27_VM4IL3	(3 << 16)
#define CSR27_VM4IL4	(4 << 16)
#define CSR27_VM4IL5	(5 << 16)
#define CSR27_VM4IL6	(6 << 16)
#define CSR27_VM4IL7	(7 << 16)
#define CSR27_VM3IL1	(1 << 12)
#define CSR27_VM3IL2	(2 << 12)
#define CSR27_VM3IL3	(3 << 12)
#define CSR27_VM3IL4	(4 << 12)
#define CSR27_VM3IL5	(5 << 12)
#define CSR27_VM3IL6	(6 << 12)
#define CSR27_VM3IL7	(7 << 12)
#define CSR27_VM2IL1	(1 << 8)
#define CSR27_VM2IL2	(2 << 8)
#define CSR27_VM2IL3	(3 << 8)
#define CSR27_VM2IL4	(4 << 8)
#define CSR27_VM2IL5	(5 << 8)
#define CSR27_VM2IL6	(6 << 8)
#define CSR27_VM2IL7	(7 << 8)
#define CSR27_VM1IL1	(1 << 4)
#define CSR27_VM1IL2	(2 << 4)
#define CSR27_VM1IL3	(3 << 4)
#define CSR27_VM1IL4	(4 << 4)
#define CSR27_VM1IL5	(5 << 4)
#define CSR27_VM1IL6	(6 << 4)
#define CSR27_VM1IL7	(7 << 4)

#define IFR0_SRQF	0x80000000

#define NVRAM_BASE      0xbfbf8000L
#define NVRAM_BYTE(n)   (*(unsigned char *)(NVRAM_BASE + ((n)*4) + 3))

#define NVRAM_ETH_ADDR(n)  NVRAM_BYTE((n)+8176)

#if 0
#define NVRAM_IP_ADDR(n)   NVRAM_BYTE((n)+8148)
#endif

#define NVRAM_TCP_PORT(n)  NVRAM_BYTE((n)+8164)

#if 0
#if defined(CHECK_FOR_NMI)
#undef CHECK_FOR_NMI
#endif

/* set k0 to non-zero if NMI */
#define	CHECK_FOR_NMI()        \
	mfc0	k0,C0_STATUS ;  \
	srl	k0,k0,20 ;     \
	andi	k0,k0,1 ;      \
	beqz	k0,1f  ;       \
	 lui	k1,0xbfbf ;    \
	li	k0,(CSR0_LB3 | CSR0_TAOUTE) >> 24 ; \
	sb	k0,0(k1) ; \
	lw	k0,%lo(CSR20)(k1) ; \
	lw	k1,%lo(CSR21)(k1) ; \
	and	k0,k0,k1 ;     \
	li	k1,CSR20_ACFIRQ|CSR20_ABTIRQ|CSR20_GP3IRQ|CSR20_SW7IRQ|CSR20_VM7IRQ ; \
	and	k0,k0,k1 ; \
     1:  nop ;
#endif

#if !defined(__ASSEMBLER__)
/* get current (24-bit) microsecond counter value */
extern unsigned int __usec_count(void);
#endif


#endif /* __DVE39_H__ */

