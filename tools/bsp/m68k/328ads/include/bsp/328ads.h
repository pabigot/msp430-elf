/*
 * 328ads.h -- Header for Motorola Dragonball (68328) ADS board.
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
#ifndef __328ADS_H__
#define __328ADS_H__ 1

#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * ROM Definitions
 */
#define ROM_ACTUAL_BASE     0x00400000
#define ROM_TOTAL_SIZE      SZ_1M
#define ROM_VIRTUAL_BASE    0x00400000

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     0x00000000
#define RAM_TOTAL_SIZE      SZ_2M
#define RAM_VIRTUAL_BASE    0x00000000

/*
 * MC68681 Register Base address
 */
#define MC68681_REG_BASE          0x00F00001
                                       
/*
 * These are offsets from the REG_BASE defined above.
 *
 * Use these in assembly w/ the appropriate REG_BASE
 * in an address register
 */
#define MC68681_MR1A_o                 0x00
#define MC68681_MR2A_o                 0x00
#define MC68681_SRA_o                  0x02
#define MC68681_CSRA_WRITE_o           0x02
#define MC68681_CSRA_READ_o            0x04
#define MC68681_CRA_o                  0x04
#define MC68681_RBA_o                  0x06
#define MC68681_TBA_o                  0x06
#define MC68681_IPCR_o                 0x08
#define MC68681_ACR_o                  0x08
#define MC68681_ISR_o                  0x0A
#define MC68681_IMR_o                  0x0A
#define MC68681_CUR_o                  0x0C
#define MC68681_CTUR_o                 0x0C
#define MC68681_CLR_o                  0x0E
#define MC68681_CTLR_o                 0x0E
#define MC68681_MR1B_o                 0x10
#define MC68681_MR2B_o                 0x10
#define MC68681_SRB_o                  0x12
#define MC68681_CSRB_WRITE_o           0x12
#define MC68681_CSRB_READ_o            0x14
#define MC68681_CRB_o                  0x14
#define MC68681_RBB_o                  0x16
#define MC68681_TBB_o                  0x16
#define MC68681_IVR_o                  0x18
#define MC68681_IP_o                   0x1A
#define MC68681_OPCR_o                 0x1A

#ifndef __ASSEMBLER__
/*
 * These are absolute addresses to the registers.
 *
 * Use these in C.
 */
#define MC68681_MR1A            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_MR1A_o)
#define MC68681_MR2A            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_MR2A_o)
#define MC68681_SRA             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_SRA_o)
#define MC68681_CSRA_WRITE      (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CSRA_WRITE_o)
#define MC68681_CSRA_READ       (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CSRA_READ_o)
#define MC68681_CRA             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CRA_o)
#define MC68681_RBA             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_RBA_o)
#define MC68681_TBA             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_TBA_o)
#define MC68681_IPCR            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_IPCR_o)
#define MC68681_ACR             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_ACR_o)
#define MC68681_ISR             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_ISR_o)
#define MC68681_IMR             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_IMR_o)
#define MC68681_CUR             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CUR_o)
#define MC68681_CTUR            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CTUR_o)
#define MC68681_CLR             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CLR_o)
#define MC68681_CTLR            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CTLR_o)
#define MC68681_MR1B            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_MR1B_o)
#define MC68681_MR2B            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_MR2B_o)
#define MC68681_SRB             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_SRB_o)
#define MC68681_CSRB_WRITE      (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CSRB_WRITE_o)
#define MC68681_CSRB_READ       (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CSRB_READ_o)
#define MC68681_CRB             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_CRB_o)
#define MC68681_RBB             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_RBB_o)
#define MC68681_TBB             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_TBB_o)
#define MC68681_IVR             (volatile unsigned char*)(MC68681_REG_BASE + MC68681_IVR_o)
#define MC68681_IP              (volatile unsigned char*)(MC68681_REG_BASE + MC68681_IP_o)
#define MC68681_OPCR            (volatile unsigned char*)(MC68681_REG_BASE + MC68681_OPCR_o)
#endif /* __ASSEMBLER__ */

#define MC68681_SR_RECEIVED_BREAK      0x80
#define MC68681_SR_FRAMING_ERROR       0x40
#define MC68681_SR_PARITY_ERROR        0x20
#define MC68681_SR_OVERRUN_ERROR       0x10
#define MC68681_SR_TXEMT               0x08
#define MC68681_SR_TXRDY               0x04
#define MC68681_SR_FFULL               0x02
#define MC68681_SR_RXRDY               0x01

#define MC68681_INT_INPUT_PORT_CHANGE  0x80
#define MC68681_INT_DELTA_BREAK_B      0x40
#define MC68681_INT_RXRDYB             0x20
#define MC68681_INT_TXRDYB             0x10
#define MC68681_INT_COUNTER_TIMER_RDY  0x08
#define MC68681_INT_DELTA_BREAK_A      0x04
#define MC68681_INT_RXRDYA             0x02
#define MC68681_INT_TXRDYA             0x01

#define MC68681_CMD_RST_MR_PTR         0x10
#define MC68681_CMD_RST_RX             0x20
#define MC68681_CMD_RST_TX             0x30
#define MC68681_CMD_RST_ERROR          0x40
#define MC68681_CMD_RST_BRK_CHANGE_INT 0x50 
#define MC68681_CMD_START_BREAK        0x60
#define MC68681_CMD_STOP_BREAK         0x70

#define MC68681_CMD_RX_ENABLED         0x01
#define MC68681_CMD_RX_DISABLED        0x02
#define MC68681_CMD_TX_ENABLED         0x04
#define MC68681_CMD_TX_DISABLED        0x08

#define MC68681_AUX_BRG_SELECT         0x80

#ifndef __ASSEMBLER__
/*
 * Define the BOARD specific data
 *
 * This currently consists of a shadow register for the DUART IMR
 */
typedef struct {
   unsigned char mc68681_imr_shadow;
} ads328_board_data;
#endif /* __ASSEMBLER__ */

#define UART_BASE_0 MC68681_MR1A
#define UART_BASE_1 MC68681_MR2A

#endif /* __328ADS_H__ */

