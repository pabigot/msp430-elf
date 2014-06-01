/*
 * yellowknife.h -- Yellowknife specific header for Cygnus BSP.
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
#ifndef __YELLOWKNIFE_H__
#define __YELLOWKNIFE_H__ 1


#define CHRP_CFG_ADDR   0xfec00000
#define CHRP_CFG_DATA   0xfee00000
#define CHRP_IO_BASE    0xfe000000
#define CHRP_ADDR_MAP   0x00000000

#define PREP_CFG_ADDR   0x80000cf8
#define PREP_CFG_DATA   0x80000cfc
#define PREP_IO_BASE    0x80000000
#define PREP_ADDR_MAP   0x00010000

#define MPC106_CFG_BASE 0x80000000

#define MPC106_CFG_ID   	0x80000000
#define MPC106_CFG_CMDSTS	0x80000004
#define MPC106_CFG_MSAR1  	0x80000080
#define MPC106_CFG_MSAR2  	0x80000084
#define MPC106_CFG_MESAR1  	0x80000088
#define MPC106_CFG_MESAR2  	0x8000008c
#define MPC106_CFG_MEAR1  	0x80000090
#define MPC106_CFG_MEAR2  	0x80000094
#define MPC106_CFG_MEEAR1  	0x80000098
#define MPC106_CFG_MEEAR2  	0x8000009c
#define MPC106_CFG_MBEN  	0x800000a0
#define MPC106_CFG_PICR1   	0x800000a8
#define MPC106_CFG_PICR2   	0x800000ac
#define MPC106_CFG_MCCR1   	0x800000f0
#define MPC106_CFG_MCCR2   	0x800000f4
#define MPC106_CFG_MCCR3   	0x800000f8
#define MPC106_CFG_MCCR4   	0x800000fc



#define MPC106_VENDOR_ID    0x1057
#define MPC106_DEVICE_ID    0x0002





#endif /* __YELLOWKNIFE_H__ */
