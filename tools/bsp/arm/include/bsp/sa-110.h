/*
 * sa-110.h -- CPU specific header for Cygnus BSP.
 *   Intel(R) SA-110
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
 *
 * Intel is a Registered Trademark of Intel Corporation.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#ifndef __SA110_H__
#define __SA110_H__ 1

#include <bsp/defs.h>

/*
 * Make sure the MMU is compiled in
 */
#define MMU

#ifdef __ASSEMBLER__

#define REG8_VAL(a)  (a)
#define REG16_VAL(a) (a)
#define REG32_VAL(a) (a)

#define REG8_PTR(a)  (a)
#define REG16_PTR(a) (a)
#define REG32_PTR(a) (a)

#else /* __ASSEMBLER__ */

#define REG8_VAL(a)  ((unsigned char)(a))
#define REG16_VAL(a) ((unsigned short)(a))
#define REG32_VAL(a) ((unsigned int)(a))

#define REG8_PTR(a)  ((volatile unsigned char *)(a))
#define REG16_PTR(a) ((volatile unsigned long *)(a))
#define REG32_PTR(a) ((volatile unsigned long *)(a))

#endif /* __ASSEMBLER__ */

/*
 * SA-110 Cache and MMU Control Registers
 *
 * Accessed through coprocessor instructions.
 */
#define SA110_ID_REGISTER                        0
#define SA110_CONTROL_REGISTER                   1
#define SA110_TRANSLATION_TABLE_BASE_REGISTER    2
#define SA110_DOMAIN_ACCESS_CONTROL_REGISTER     3
#define SA110_FAULT_STATUS_REGISTER              5
#define SA110_FAULT_ADDRESS_REGISTER             6
#define SA110_CACHE_OPERATIONS_REGISTER          7
#define SA110_TLB_OPERATIONS_REGISTER            8
#define SA110_TEST_CLOCK_AND_IDLE_REGISTER       15

/*
 * SA-110 Cache and MMU Definitions
 */
#define SA110_ICACHE_SIZE                       SZ_16K
#define SA110_DCACHE_SIZE                       SZ_16K
#define SA110_ICACHE_LINESIZE_BYTES             32
#define SA110_DCACHE_LINESIZE_BYTES             32
#define SA110_ICACHE_LINESIZE_WORDS             8
#define SA110_DCACHE_LINESIZE_WORDS             8
#define SA110_ICACHE_WAYS                       32
#define SA110_DCACHE_WAYS                       32
#define SA110_ICACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(SA110_ICACHE_LINESIZE_BYTES - 1))
#define SA110_DCACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(SA110_DCACHE_LINESIZE_BYTES - 1))

#define SA110_ZEROS_BANK_BASE                  (0x50000000)

/*
 * SA-110 Cache and MMU ID Register value
 */
#define SA110_ID_MASK                            0xFFFFFFF0
#define SA110_ID_VALUE                           0x4401a100

/*
 * SA-110 Cache Control Register Bit Fields and Masks
 */
#define SA110_MMU_DISABLED                       0x00000000
#define SA110_MMU_ENABLED                        0x00000001
#define SA110_MMU_MASK                           0x00000001
#define SA110_ADDRESS_FAULT_DISABLED             0x00000000
#define SA110_ADDRESS_FAULT_ENABLED              0x00000002
#define SA110_ADDRESS_FAULT_MASK                 0x00000002
#define SA110_DATA_CACHE_DISABLED                0x00000000
#define SA110_DATA_CACHE_ENABLED                 0x00000004
#define SA110_DATA_CACHE_MASK                    0x00000004
#define SA110_WRITE_BUFFER_DISABLED              0x00000000
#define SA110_WRITE_BUFFER_ENABLED               0x00000008
#define SA110_WRITE_BUFFER_MASK                  0x00000008
#define SA110_LITTLE_ENDIAN                      0x00000000
#define SA110_BIG_ENDIAN                         0x00000080
#define SA110_ACCESS_CHECKS_SYSTEM               0x00000100
#define SA110_ACCESS_CHECKS_ROM                  0x00000200
#define SA110_INSTRUCTION_CACHE_DISABLED         0x00000000
#define SA110_INSTRUCTION_CACHE_ENABLED          0x00001000
#define SA110_INSTRUCTION_CACHE_MASK             0x00001000

/*
 * SA-110 Translation Table Base Bit Masks
 */
#define SA110_TRANSLATION_TABLE_MASK             0xFFFFC000

/*
 * SA-110 Domain Access Control Bit Masks
 */
#define SA110_DOMAIN_0_MASK                      0x00000003
#define SA110_DOMAIN_1_MASK                      0x0000000C
#define SA110_DOMAIN_2_MASK                      0x00000030
#define SA110_DOMAIN_3_MASK                      0x000000C0
#define SA110_DOMAIN_4_MASK                      0x00000300
#define SA110_DOMAIN_5_MASK                      0x00000C00
#define SA110_DOMAIN_6_MASK                      0x00003000
#define SA110_DOMAIN_7_MASK                      0x0000C000
#define SA110_DOMAIN_8_MASK                      0x00030000
#define SA110_DOMAIN_9_MASK                      0x000C0000
#define SA110_DOMAIN_10_MASK                     0x00300000
#define SA110_DOMAIN_11_MASK                     0x00C00000
#define SA110_DOMAIN_12_MASK                     0x03000000
#define SA110_DOMAIN_13_MASK                     0x0C000000
#define SA110_DOMAIN_14_MASK                     0x30000000
#define SA110_DOMAIN_15_MASK                     0xC0000000

/*
 * SA-110 Fault Status Bit Masks
 */
#define SA110_FAULT_STATUS_MASK                  0x0000000F
#define SA110_DOMAIN_MASK                        0x000000F0

/*
 * SA-110 Cache Control Operations Definitions
 */
#define SA110_FLUSH_CACHE_INST_DATA_OPCODE       0x0
#define SA110_FLUSH_CACHE_INST_DATA_RM           0x7
#define SA110_FLUSH_CACHE_INST_OPCODE            0x0
#define SA110_FLUSH_CACHE_INST_RM                0x5
#define SA110_FLUSH_CACHE_DATA_OPCODE            0x0
#define SA110_FLUSH_CACHE_DATA_RM                0x6
#define SA110_FLUSH_CACHE_DATA_SINGLE_OPCODE     0x1
#define SA110_FLUSH_CACHE_DATA_SINGLE_RM         0x6
#define SA110_CLEAN_CACHE_DATA_ENTRY_OPCODE      0x1
#define SA110_CLEAN_CACHE_DATA_ENTRY_RM          0xA
#define SA110_DRAIN_CACHE_WRITE_BUFFER_OPCODE    0x4
#define SA110_DRAIN_CACHE_WRITE_BUFFER_RM        0xA

/*
 * SA-110 TLB Operations Definitions
 */
#define SA110_FLUSH_INST_DATA_TLB_OPCODE         0x0
#define SA110_FLUSH_INST_DATA_TLB_RM             0x7
#define SA110_FLUSH_INST_TLB_OPCODE              0x0
#define SA110_FLUSH_INST_TLB_RM                  0x5
#define SA110_FLUSH_DATA_TLB_OPCODE              0x0
#define SA110_FLUSH_DATA_TLB_RM                  0x6
#define SA110_FLUSH_DATA_ENTRY_TLB_OPCODE        0x1
#define SA110_FLUSH_DATA_ENTRY_TLB_RM            0x6

/*
 * SA-110 Test, Clock and Idle Control Definition
 */
#define SA110_ICACHE_ODD_WORD_LOADING_OPCODE     0x1
#define SA110_ICACHE_ODD_WORD_LOADING_RM         0x1
#define SA110_ICACHE_EVEN_WORD_LOADING_OPCODE    0x1
#define SA110_ICACHE_EVEN_WORD_LOADING_RM        0x2
#define SA110_ICACHE_CLEAR_LFSR_OPCODE           0x1
#define SA110_ICACHE_CLEAR_LFSR_RM               0x4
#define SA110_ENABLE_CLOCK_SWITCHING_OPCODE      0x2
#define SA110_ENABLE_CLOCK_SWITCHING_RM          0x1
#define SA110_DISABLE_CLOCK_SWITCHING_OPCODE     0x2
#define SA110_DISABLE_CLOCK_SWITCHING_RM         0x2
#define SA110_DISABLE_mCLK_OUTPUT_OPCODE         0x2
#define SA110_DISABLE_mCLK_OUTPUT_RM             0x4
#define SA110_WAIT_FOR_INTERRUPT_OPCODE          0x2
#define SA110_WAIT_FOR_INTERRUPT_RM              0x8


/*
 * SA-110 Control and Status Register Base Definitions
 */
#define SA110_CONTROL_STATUS_BASE                0x42000000
#define SA110_REGISTER(x)                        REG32_PTR(SA110_CONTROL_STATUS_BASE + (x))

/*
 * PCI Control and Status Registers
 */
#define SA110_OUT_INT_STATUS_o			0x30
#define SA110_OUT_INT_MASK_o			0x34
#define SA110_PCI_INT_LINE_o			0x3c
#define SA110_INBOUND_FIFO_o			0x40
#define SA110_OUTBOUND_FIFO_o			0x44
#define SA110_MAILBOX0_o			0x50
#define SA110_MAILBOX1_o			0x54
#define SA110_MAILBOX2_o			0x58
#define SA110_MAILBOX3_o			0x5C
#define SA110_DOORBELL_o			0x60
#define SA110_DOORBELL_SETUP_o			0x64

#define SA110_PCI_ADDR_EXT_o		0x150
#define SA110_DOORBELL_PCI_MASK_o		0x150
#define SA110_DOORBELL_SA_MASK_o		0x154

/*
 * SA-110 CSR Register Definitions
 */
#define SA110_CSR_BASE_ADDRESS_MASK_o            0xF8
#define SA110_CSR_BASE_ADDRESS_OFFSET_o          0xFC

#define SA110_CSR_BASE_ADDRESS_MASK              SA110_REGISTER(SA110_CSR_BASE_ADDRESS_MASK_o)
#define SA110_CSR_BASE_ADDRESS_OFFSET            SA110_REGISTER(SA110_CSR_BASE_ADDRESS_OFFSET_o)

/*
 * SA-110 CSR Register Value Definitions
 */
#define SA110_CSR_WINDOW_SIZE_128                0x00000000
#define SA110_CSR_WINDOW_SIZE_512KB              0x00040000
#define SA110_CSR_WINDOW_SIZE_1MB                0x000C0000
#define SA110_CSR_WINDOW_SIZE_2MB                0x001C0000
#define SA110_CSR_WINDOW_SIZE_4MB                0x003C0000
#define SA110_CSR_WINDOW_SIZE_8MB                0x007C0000
#define SA110_CSR_WINDOW_SIZE_16MB               0x00FC0000
#define SA110_CSR_WINDOW_SIZE_32MB               0x01FC0000
#define SA110_CSR_WINDOW_SIZE_64MB               0x03FC0000
#define SA110_CSR_WINDOW_SIZE_128MB              0x07FC0000
#define SA110_CSR_WINDOW_SIZE_256MB              0x0FFC0000

/*
 * SA-110 SDRAM Register Definitions
 */
#define SA110_SDRAM_ARRAY_0_MODE_REGISTER_BASE   REG32_PTR(0x40000000)
#define SA110_SDRAM_ARRAY_1_MODE_REGISTER_BASE   REG32_PTR(0x40004000)
#define SA110_SDRAM_ARRAY_2_MODE_REGISTER_BASE   REG32_PTR(0x40008000)
#define SA110_SDRAM_ARRAY_3_MODE_REGISTER_BASE   REG32_PTR(0x4000C000)

#define SA110_SDRAM_BASE_ADDRESS_MASK_o          0x100
#define SA110_SDRAM_BASE_ADDRESS_OFFSET_o        0x104
#define SA110_EXP_ROM_BASE_ADDRESS_MASK_o        0x108
#define SA110_SDRAM_TIMING_o                     0x10C
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o       0x110
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_1_o       0x114
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_2_o       0x118
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o       0x11C

#define SA110_SDRAM_BASE_ADDRESS_MASK            SA110_REGISTER(SA110_SDRAM_BASE_ADDRESS_MASK_o)
#define SA110_SDRAM_BASE_ADDRESS_OFFSET          SA110_REGISTER(SA110_SDRAM_BASE_ADDRESS_OFFSET_o)
#define SA110_EXP_ROM_BASE_ADDRESS_MASK          SA110_REGISTER(SA110_EXP_ROM_BASE_ADDRESS_MASK_o)
#define SA110_SDRAM_TIMING                       SA110_REGISTER(SA110_SDRAM_TIMING_o)
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_0         SA110_REGISTER(SA110_SDRAM_ADDRESS_SIZE_ARRAY_0_o)
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_1         SA110_REGISTER(SA110_SDRAM_ADDRESS_SIZE_ARRAY_1_o)
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_2         SA110_REGISTER(SA110_SDRAM_ADDRESS_SIZE_ARRAY_2_o)
#define SA110_SDRAM_ADDRESS_SIZE_ARRAY_3         SA110_REGISTER(SA110_SDRAM_ADDRESS_SIZE_ARRAY_3_o)

#define SA110_SDRAM_SIZE_1M			1
#define SA110_SDRAM_SIZE_2M			2
#define SA110_SDRAM_SIZE_4M			3
#define SA110_SDRAM_SIZE_8M			4
#define SA110_SDRAM_SIZE_16M			5
#define SA110_SDRAM_SIZE_32M			6
#define SA110_SDRAM_SIZE_64M			7

#define SA110_SDRAM_MUX_MODE0			0x00
#define SA110_SDRAM_MUX_MODE1			0x10
#define SA110_SDRAM_MUX_MODE2			0x20
#define SA110_SDRAM_MUX_MODE3			0x30
#define SA110_SDRAM_MUX_MODE4			0x40
#define SA110_SDRAM_MUX_MODE_MASK		0x70



/*
 * SA-110 SDRAM Configuration Value Definitions
 */
#define SA110_SDRAM_WINDOW_SIZE_256KB            0x00000000
#define SA110_SDRAM_WINDOW_SIZE_512KB            0x00040000
#define SA110_SDRAM_WINDOW_SIZE_1MB              0x000C0000
#define SA110_SDRAM_WINDOW_SIZE_2MB              0x001C0000
#define SA110_SDRAM_WINDOW_SIZE_4MB              0x003C0000
#define SA110_SDRAM_WINDOW_SIZE_8MB              0x007C0000
#define SA110_SDRAM_WINDOW_SIZE_16MB             0x00FC0000
#define SA110_SDRAM_WINDOW_SIZE_32MB             0x01FC0000
#define SA110_SDRAM_WINDOW_SIZE_64MB             0x03FC0000
#define SA110_SDRAM_WINDOW_SIZE_128MB            0x07FC0000
#define SA110_SDRAM_WINDOW_SIZE_256MB            0x0FFC0000
#define SA110_SDRAM_WINDOW_SIZE_NO_WINDOW        0x8FFC0000

/*
 * SA-110 Expansion ROM Configuration Value Definitions
 */
#define SA110_EXP_ROM_WINDOW_SIZE_1MB            0x00000000
#define SA110_EXP_ROM_WINDOW_SIZE_2MB            0x00100000
#define SA110_EXP_ROM_WINDOW_SIZE_4MB            0x00300000
#define SA110_EXP_ROM_WINDOW_SIZE_8MB            0x00700000
#define SA110_EXP_ROM_WINDOW_SIZE_16MB           0x00F00000
#define SA110_EXP_ROM_WINDOW_SIZE_NO_WINDOW      0x80F00000

/*
 * SA-110 SDRAM Timing Register Field Definitions
 */
#define SA110_SDRAM_ROW_PRECHARGE_1_CYCLE        0x00000000
#define SA110_SDRAM_ROW_PRECHARGE_2_CYCLES       0x00000001
#define SA110_SDRAM_ROW_PRECHARGE_3_CYCLES       0x00000002
#define SA110_SDRAM_ROW_PRECHARGE_4_CYCLES       0x00000003
#define SA110_SDRAM_LAST_DATA_IN_2_CYCLES        0x00000000
#define SA110_SDRAM_LAST_DATA_IN_3_CYCLES        0x00000004
#define SA110_SDRAM_LAST_DATA_IN_4_CYCLES        0x00000008
#define SA110_SDRAM_LAST_DATA_IN_5_CYCLES        0x0000000C

#define SA110_SDRAM_RAS_TO_CAS_DELAY_2_CYCLES    0x00000020
#define SA110_SDRAM_RAS_TO_CAS_DELAY_3_CYCLES    0x00000030
#define SA110_SDRAM_CAS_LATENCY_2_CYCLES         0x00000080
#define SA110_SDRAM_CAS_LATENCY_3_CYCLES         0x000000C0

#define SA110_SDRAM_ROW_CYCLE_TIME_4_CYCLES      0x00000100
#define SA110_SDRAM_ROW_CYCLE_TIME_5_CYCLES      0x00000200
#define SA110_SDRAM_ROW_CYCLE_TIME_6_CYCLES      0x00000300
#define SA110_SDRAM_ROW_CYCLE_TIME_7_CYCLES      0x00000400
#define SA110_SDRAM_ROW_CYCLE_TIME_8_CYCLES      0x00000500
#define SA110_SDRAM_ROW_CYCLE_TIME_9_CYCLES      0x00000600
#define SA110_SDRAM_ROW_CYCLE_TIME_10_CYCLES     0x00000700

#define SA110_SDRAM_COMMAND_DRIVE_SAME_CYCLE     0x00000000
#define SA110_SDRAM_COMMAND_DRIVE_1_CYCLE        0x00000800

#define SA110_SDRAM_PARITY_DISABLED              0x00000000
#define SA110_SDRAM_PARITY_ENABLED               0x00001000
#define SA110_SDRAM_PARITY_MASK                  0x00001000

#define SA110_SDRAM_SA110_PRIME_DISABLED         0x00000000
#define SA110_SDRAM_SA110_PRIME_ENABLED          0x00002000
#define SA110_SDRAM_SA110_PRIME_MASK             0x00002000

#define SA110_SDRAM_REFRESH_INTERVAL(x)          (((x) << 16) & 0x003f0000)
#define SA110_SDRAM_REFRESH_INTERVAL_MIN         SA110_SDRAM_REFRESH_INTERVAL(1)
#define SA110_SDRAM_REFRESH_INTERVAL_NORMAL      SA110_SDRAM_REFRESH_INTERVAL(0x1A)

/*
 * SA-110 SDRAM Address and Size Register Field Definitions
 */
#define SA110_SDRAM_SIZE_0                       0x000000000
#define SA110_SDRAM_SIZE_1MB                     0x000000001
#define SA110_SDRAM_SIZE_2MB                     0x000000002
#define SA110_SDRAM_SIZE_4MB                     0x000000003
#define SA110_SDRAM_SIZE_8MB                     0x000000004
#define SA110_SDRAM_SIZE_16MB                    0x000000005
#define SA110_SDRAM_SIZE_32MB                    0x000000006
#define SA110_SDRAM_SIZE_64MB                    0x000000007

#define SA110_SDRAM_ADDRESS_MULTIPLEX_MASK       0x000000070
#define SA110_SDRAM_ARRAY_BASE_MASK              0x00FF00000

/*
 * SA-110 Control Register.
 */
#define SA110_CONTROL_o                          0x13C

#define SA110_CONTROL                            SA110_REGISTER(SA110_CONTROL_o)

/*
 * Control bits.
 */
#define SA110_CONTROL_INIT_COMPLETE	0x00000001
#define SA110_CONTROL_CFN		0x80000000

/*
 * SA-110 UART Control/Configuration Registers.
 */
#define SA110_UART_DATA_REGISTER_o               0x160
#define SA110_UART_RXSTAT_o                      0x164
#define SA110_UART_H_BAUD_CONTROL_o              0x168
#define SA110_UART_M_BAUD_CONTROL_o              0x16C
#define SA110_UART_L_BAUD_CONTROL_o              0x170
#define SA110_UART_CONTROL_REGISTER_o            0x174
#define SA110_UART_FLAG_REGISTER_o               0x178

#define SA110_UART_DATA_REGISTER                 SA110_REGISTER(SA110_UART_DATA_REGISTER_o)
#define SA110_UART_RXSTAT                        SA110_REGISTER(SA110_UART_RXSTAT_o)
#define SA110_UART_H_BAUD_CONTROL                SA110_REGISTER(SA110_UART_H_BAUD_CONTROL_o)
#define SA110_UART_M_BAUD_CONTROL                SA110_REGISTER(SA110_UART_M_BAUD_CONTROL_o)
#define SA110_UART_L_BAUD_CONTROL                SA110_REGISTER(SA110_UART_L_BAUD_CONTROL_o)
#define SA110_UART_CONTROL_REGISTER              SA110_REGISTER(SA110_UART_CONTROL_REGISTER_o)
#define SA110_UART_FLAG_REGISTER                 SA110_REGISTER(SA110_UART_FLAG_REGISTER_o)

#define UART_BASE_0                              SA110_UART_DATA_REGISTER

/*
 * SA-110 UART Data Register bit masks
 */
#define SA110_UART_DATA_MASK                     0x000000FF

/*
 * SA-110 UART RX Status Register bit masks
 */
#define SA110_UART_FRAMING_ERROR_MASK            0x00000001
#define SA110_UART_PARITY_ERROR_MASK             0x00000002
#define SA110_UART_OVERRUN_ERROR_MASK            0x00000004

/*
 * SA-110 UART High Baud Control Register bit masks
 */
#define SA110_UART_BREAK_DISABLED                0x00000000
#define SA110_UART_BREAK_ENABLED                 0x00000001
#define SA110_UART_BREAK_MASK                    0x00000001
#define SA110_UART_PARITY_DISABLED               0x00000000
#define SA110_UART_PARITY_ENABLED                0x00000002
#define SA110_UART_PARITY_MASK                   0x00000002
#define SA110_UART_PARITY_ODD                    0x00000000
#define SA110_UART_PARITY_EVEN                   0x00000004
#define SA110_UART_ODD_EVEN_SELECT_MASK          0x00000004
#define SA110_UART_STOP_BITS_ONE                 0x00000000
#define SA110_UART_STOP_BITS_TWO                 0x00000008
#define SA110_UART_STOP_BITS_SELECT_MASK         0x00000008
#define SA110_UART_FIFO_DISABLED                 0x00000000
#define SA110_UART_FIFO_ENABLED                  0x00000010
#define SA110_UART_FIFO_ENABLE_MASK              0x00000010
#define SA110_UART_DATA_LENGTH_5_BITS            0x00000000
#define SA110_UART_DATA_LENGTH_6_BITS            0x00000020
#define SA110_UART_DATA_LENGTH_7_BITS            0x00000040
#define SA110_UART_DATA_LENGTH_8_BITS            0x00000060
#define SA110_UART_DATA_LENGTH_MASK              0x00000060

/*
 * SA-110 UART Medium Baud Control Register bit masks
 */
#define SA110_UART_H_BAUD_RATE_DIVISOR_MASK      0x0000000F

/*
 * SA-110 UART Low Baud Control Register bit masks
 */
#define SA110_UART_L_BAUD_RATE_DIVISOR_MASK      0x000000FF

/*
 * SA-110 UART Control Register bit fields
 */
#define SA110_UART_DISABLED                      0x00000000
#define SA110_UART_ENABLED                       0x00000001
#define SA110_UART_ENABLE_MASK                   0x00000001
#define SA110_SIR_DISABLED                       0x00000000
#define SA110_SIR_ENABLED                        0x00000002
#define SA110_SIR_ENABLE_MASK                    0x00000002
#define SA110_SIR_PULSE_WIDTH_BIT_RATE           0x00000000
#define SA110_SIR_PULSE_WIDTH_MAX_CLK            0x00000004

/*
 * SA-110 UART Flag Register bit masks
 */
#define SA110_TX_IDLE                            0x00000000
#define SA110_TX_BUSY                            0x00000008
#define SA110_TX_BUSY_MASK                       0x00000008
#define SA110_RX_FIFO_FULL                       0x00000000
#define SA110_RX_FIFO_EMPTY                      0x00000010
#define SA110_RX_FIFO_STATUS_MASK                0x00000010
#define SA110_TX_FIFO_READY                      0x00000000
#define SA110_TX_FIFO_BUSY                       0x00000020
#define SA110_TX_FIFO_STATUS_MASK                0x00000020

/*
 * SA-110 IRQ Controller Registers
 */
#define SA110_IRQCONT_IRQSTATUS_o                0x180
#define SA110_IRQCONT_IRQRAWSTATUS_o             0x184
#define SA110_IRQCONT_IRQENABLE_o                0x188
#define SA110_IRQCONT_IRQENABLESET_o             0x188
#define SA110_IRQCONT_IRQENABLECLEAR_o           0x18C
#define SA110_IRQCONT_IRQSOFT_o                  0x190
#define SA110_IRQCONT_FIQSTATUS_o                0x280
#define SA110_IRQCONT_FIQRAWSTATUS_o             0x284
#define SA110_IRQCONT_FIQENABLE_o                0x288
#define SA110_IRQCONT_FIQENABLESET_o             0x288
#define SA110_IRQCONT_FIQENABLECLEAR_o           0x28C
#define SA110_IRQCONT_FIQSOFT_o                  0x290

#define SA110_IRQCONT_IRQSTATUS                  SA110_REGISTER(SA110_IRQCONT_IRQSTATUS_o)
#define SA110_IRQCONT_IRQRAWSTATUS               SA110_REGISTER(SA110_IRQCONT_IRQRAWSTATUS_o)
#define SA110_IRQCONT_IRQENABLE                  SA110_REGISTER(SA110_IRQCONT_IRQENABLE_o)
#define SA110_IRQCONT_IRQENABLESET               SA110_REGISTER(SA110_IRQCONT_IRQENABLESET_o)
#define SA110_IRQCONT_IRQENABLECLEAR             SA110_REGISTER(SA110_IRQCONT_IRQENABLECLEAR_o)
#define SA110_IRQCONT_IRQSOFT                    SA110_REGISTER(SA110_IRQCONT_IRQSOFT_o)
#define SA110_IRQCONT_FIQSTATUS                  SA110_REGISTER(SA110_IRQCONT_FIQSTATUS_o)
#define SA110_IRQCONT_FIQRAWSTATUS               SA110_REGISTER(SA110_IRQCONT_FIQRAWSTATUS_o)
#define SA110_IRQCONT_FIQENABLE                  SA110_REGISTER(SA110_IRQCONT_FIQENABLE_o)
#define SA110_IRQCONT_FIQENABLESET               SA110_REGISTER(SA110_IRQCONT_FIQENABLESET_o)
#define SA110_IRQCONT_FIQENABLECLEAR             SA110_REGISTER(SA110_IRQCONT_FIQENABLECLEAR_o)
#define SA110_IRQCONT_FIQSOFT                    SA110_REGISTER(SA110_IRQCONT_FIQSOFT_o)

/*
 * IRQ Controller IRQ Numbers
 */
#define SA110_IRQ_MIN                            0

#define SA110_IRQ_RSV0                           0
#define SA110_IRQ_SOFT_INTERRUPT                 1
#define SA110_IRQ_CONSOLE_RX                     2
#define SA110_IRQ_CONSOLE_TX                     3
#define SA110_IRQ_TIMER_1                        4
#define SA110_IRQ_TIMER_2                        5
#define SA110_IRQ_TIMER_3                        6
#define SA110_IRQ_TIMER_4                        7
#define SA110_IRQ_IRQ_IN_I_0                     8
#define SA110_IRQ_IRQ_IN_I_1                     9
#define SA110_IRQ_IRQ_IN_I_2                     10
#define SA110_IRQ_IRQ_IN_I_3                     11
#define SA110_IRQ_XCS_I_0                        12
#define SA110_IRQ_XCS_I_1                        13
#define SA110_IRQ_XCS_I_2                        14
#define SA110_IRQ_DOORBELL_FROM_HOST             15
#define SA110_IRQ_DMA_CHAN_1                     16
#define SA110_IRQ_DMA_CHAN_2                     17
#define SA110_IRQ_PIC_IRQ_I                      18
#define SA110_IRQ_PMCSR_WRITE_BY_HOST            19
#define SA110_IRQ_RSV1                           20
#define SA110_IRQ_RSV2                           21
#define SA110_IRQ_START_BIST                     22
#define SA110_IRQ_RECEIVED_SERR                  23
#define SA110_IRQ_SDRAM_PARITY                   24
#define SA110_IRQ_I20_INBOUND_POST_LIST          25
#define SA110_IRQ_RSV3                           26
#define SA110_IRQ_DISCARD_TIMER_EXPIRED          27
#define SA110_IRQ_DATA_PARITY_ERROR              28
#define SA110_IRQ_MASTER_ABORT                   29
#define SA110_IRQ_TARGET_ABORT                   30
#define SA110_IRQ_PARITY_ERROR                   31

#define SA110_IRQ_MAX                            31
#define NUM_SA110_INTERRUPTS                     SA110_IRQ_MAX - SA110_IRQ_MIN + 1
#define SA110_IRQ_INTSRC_MASK(irq_nr)            (1 << (irq_nr))


/*
 * SA110 IRQSOFT/FIQSOFT Register bit fields
 */
#define SA110_IRQSOFT_RAW_BIT_MASK               0x00000001

/*
 * SA-110 Miscellaneous Registers.
 */
#define SA110_XBUS_CYCLE_ARBITER_o               0x148
#define SA110_XBUS_IO_STROBE_MASK_o              0x14c

#define SA110_XBUS_CYCLE_ARBITER                 SA110_REGISTER(SA110_XBUS_CYCLE_ARBITER_o)
#define SA110_XBUS_IO_STROBE_MASK                SA110_REGISTER(SA110_XBUS_IO_STROBE_MASK_o)



#endif /* __SA110_H__ */
