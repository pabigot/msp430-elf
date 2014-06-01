/*
 * fr500.h -- Definitions for Fujitsu FR-V family FR500 Series CPUs.
 *
 * Copyright (c) 1999, 2000 Red Hat, Inc.
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
#ifndef __FR500_H__
#define __FR500_H__ 1

/*
 * System Control Registers.
 */
#define SYSCTL_TCBCFG  0xfeff0400
#define SYSCTL_RCBCFG  0xfeff0408
#define SYSCTL_TWDBCFG 0xfeff0410
#define SYSCTL_TRDBCFG 0xfeff0418
#define SYSCTL_RDBCFG  0xfeff0420
#define SYSCTL_AEFR    0xfeff0428

/*
 * Companion Chip Registers.
 */
#ifdef __CPU_TOMCAT__
#define COMP_CIRR	0xfeff8000
#define COMP_TCBCFG	0xfeff8400
#define COMP_RCBCFG	0xfeff8408
#define COMP_RWDBCFG	0xfeff8410
#define COMP_RRDBCFG	0xfeff8418
#define COMP_TDBCFG	0xfeff8420
#define COMP_AEFR	0xfeff8440
#define COMP_AEAR	0xfeff8448
#define COMP_AEFMR	0xfeff8450
#define COMP_ISR	0xfeff8458
#define COMP_BOOTCTL	0xfeff8460
#endif

#ifdef __CPU_TIGER__
#define COMP_PCNTL	0xfeff8000
#define COMP_RCBCFG	0xfeff8400
#define COMP_TCBCFG	0xfeff8408
#define COMP_RWDBCFG	0xfeff8410
#define COMP_RRDBCFG	0xfeff8418
#define COMP_TDBCFG	0xfeff8420
#define COMP_AEFR	0xfeff8440
#define COMP_AEFMR	0xfeff8448
#define COMP_AEAR	0xfeff8450
#define COMP_BOOTCTL	0xfeff8460

#define COMP_ARSR1	0xfeff8808
#define COMP_ARSR2	0xfeff8810
#define COMP_ARSR3	0xfeff8818
#define COMP_ARSR4	0xfeff8820
#define COMP_ARSR5	0xfeff8828

#define COMP_AMR1	0xfeff8838
#define COMP_AMR2	0xfeff8840
#define COMP_AMR3	0xfeff8848
#define COMP_AMR4	0xfeff8850
#define COMP_AMR5	0xfeff8858

#define COMP_WSSR0	0xfeff8860
#define COMP_WSSR1	0xfeff8868
#define COMP_WSSR2	0xfeff8870
#define COMP_WSSR3	0xfeff8878
#define COMP_WSSR4	0xfeff8880
#define COMP_WSSR5	0xfeff8888

#define COMP_ICCR	0xfeff8890
#define COMP_MXEF	0xfeff8898
#define COMP_MEXECR	0xfeff88a0
#define COMP_MAEAR	0xfeff88a8
#define COMP_EMAEAR	0xfeff88b0

#define COMP_TCSR0	0xfeff9400
#define COMP_TCSR1	0xfeff9408
#define COMP_TCSR2	0xfeff9410
#define COMP_TCTR	0xfeff9418
#define COMP_TPRV	0xfeff9420
#define COMP_TPRCKSL	0xfeff9428
#define COMP_T0CKSL	0xfeff9430
#define COMP_T1CKSL	0xfeff9438
#define COMP_T2CKSL	0xfeff9440
#define COMP_TM0	0xfeff9800
#define COMP_TM1	0xfeff9808
#define COMP_RS		0xfeff9810
#define COMP_RC		0xfeff9818
#define COMP_MASK	0xfeff9820
#define COMP_IRL	0xfeff9828
#define COMP_IRR0	0xfeff9840
#define COMP_IRR1	0xfeff9848
#define COMP_IRR2	0xfeff9850
#define COMP_IRR3	0xfeff9858
#define COMP_IRR4	0xfeff9860
#define COMP_IRR5	0xfeff9868
#define COMP_IRR6	0xfeff9870
#define COMP_IRR7	0xfeff9878
#define COMP_IITMR0	0xfeff9880
#define COMP_IITMR1	0xfeff9888
#endif

#define COMP_LMXEF	0xfeffa080
#define COMP_LMXEMR	0xfeffa088
#define COMP_LMAEAR	0xfeffa090
#define COMP_LEMAEAR	0xfeffa098
#define COMP_LWTCR	0xfeffa0a0

/*
 * SDRAM Controller Registers.
 */
#define SDRAM_DARS0	0xfeff0100
#define SDRAM_DARS1	0xfeff0104
#define SDRAM_DARS2	0xfeff0108
#define SDRAM_DARS3	0xfeff010c
#define SDRAM_DAMK0	0xfeff0110
#define SDRAM_DAMK1	0xfeff0114
#define SDRAM_DAMK2	0xfeff0118
#define SDRAM_DAMK3	0xfeff011c

#define SDRAM_DCTL	0xfeff0200
#define DCTL_TRAS_5     (5 << 17)
#define DCTL_TRAS_6     (6 << 17)
#define DCTL_TRP_2      (2 << 13)
#define DCTL_TRP_3      (3 << 13)
#define DCTL_TRCD_2     (2 <<  8)
#define DCTL_TRCD_3     (3 <<  8)
#define DCTL_TRRD_2     (2 <<  4)
#define DCTL_TRRD_3     (3 <<  4)
#define DCTL_TDPL_0     (1 <<  2)
#define DCTL_TDPL_1     (2 <<  2)
#define DCTL_TWR_0      (0 <<  0)

#define SDRAM_DAMC	0xfeff0204
#define SDRAM_DMS	0xfeff0208
#define DMS_CL_2	(2 << 12)
#define DMS_CL_3	(3 << 12)
#define DMS_BT 		(1 << 11)
#define DMS_BL_8	(3 <<  8)
#define DMS_WM 		(1 <<  0)

#define SDRAM_DCFG	0xfeff020c
#define DCFG_64BIT	(0 << 2)
#define DCFG_32BIT	(1 << 2)
#define DCFG_SDRAM	(0 << 0)
#define DCFG_DIMM	(1 << 0)

#define SDRAM_DAN	0xfeff0210

#define SDRAM_DSTS	0xfeff0214
#define DSTS_SSI        (1 << 1)
#define DSTS_SDI	(1 << 0)

#define SDRAM_DRCN	0xfeff0218
#define DRCN_BRE	(1 << 15)
#define DRCN_ARS_4K	(0 << 13)
#define DRCN_ARS_8K	(1 << 13)
#define DRCN_ARS_2K	(3 << 13)
#define DRCN_SRN_1X	(0 << 11)
#define DRCN_SRN_4X	(1 << 11)
#define DRCN_SRE	(1 << 9)
#define DRCN_RE		(1 << 8)
#define DRCN_SRENT	(1 << 0)

#define SDRAM_DART	0xfeff021c


#ifdef __CPU_TIGER__
#ifndef __ASSEMBLER__
/* Interrupt trigger modes. */
#define TRIG_LEVEL_H   0   /* trigger on high level   */
#define TRIG_LEVEL_L   1   /* trigger on low level    */
#define TRIG_EDGE_H    2   /* trigger on rising edge  */
#define TRIG_EDGE_L    3   /* trigger on falling edge */


static inline unsigned
comp_read(int regnum)
{
    return *(unsigned *)regnum;
}


static inline void
comp_write(int regnum, unsigned val)
{
    *(unsigned *)regnum = val;
}
#endif
#endif


/* Debug Support Unit */

/*
 * Debug Control Register
 */
#define DCR_EBE		(1 << 30)
#define DCR_SE		(1 << 29)
#define DCR_DRBE0	(1 << 19)
#define DCR_DWBE0	(1 << 18)
#define DCR_DDBE0	(1 << 17)
#define DCR_DRBE1	(1 << 16)
#define DCR_DWBE1	(1 << 15)
#define DCR_DDBE1	(1 << 14)
#define DCR_DRBE2	(1 << 13)
#define DCR_DWBE2	(1 << 12)
#define DCR_DDBE2	(1 << 11)
#define DCR_DRBE3	(1 << 10)
#define DCR_DWBE3	(1 << 9)
#define DCR_DDBE3	(1 << 8)
#define DCR_IBE0	(1 << 7)
#define DCR_IBCE0	(1 << 6)
#define DCR_IBE1	(1 << 5)
#define DCR_IBCE1	(1 << 4)
#define DCR_IBE2	(1 << 3)
#define DCR_IBCE2	(1 << 2)
#define DCR_IBE3	(1 << 1)
#define DCR_IBCE3	(1 << 0)


/*
 * Break Request Register
 */
#define BRR_EB		(1 << 30)
#define BRR_CB		(1 << 29)
#define BRR_TB		(1 << 28)
#define BRR_EBTT_MASK	0xff
#define BRR_EBTT_SHIFT	16
#define BRR_DBNE0	(1 << 15)
#define BRR_DBNE1	(1 << 14)
#define BRR_DBNE2	(1 << 13)
#define BRR_DBNE3	(1 << 12)
#define BRR_DB0		(1 << 11)
#define BRR_DB1		(1 << 10)
#define BRR_DB2		(1 << 9)
#define BRR_DB3		(1 << 8)
#define BRR_IB0		(1 << 7)
#define BRR_IB1		(1 << 6)
#define BRR_IB2		(1 << 5)
#define BRR_IB3		(1 << 4)
#define BRR_SB		(1 << 1)
#define BRR_ST		(1 << 0)


#endif /* __FR500_H__ */

