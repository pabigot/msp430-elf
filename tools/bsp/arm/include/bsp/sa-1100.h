/*
 * sa-1100.h -- CPU specific header for Cygnus BSP.
 *   Intel(R) SA-1100
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
 *
 * Intel is a Registered Trademark of Intel Corporation.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#ifndef __SA1100_H__
#define __SA1100_H__ 1

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
 * SA1100 Default Memory Layout Definitions
 */
#define SA1100_ROM_BANK0_BASE                    (0)
#define SA1100_ROM_BANK1_BASE                    (SA1100_ROM_BANK0_BASE + SZ_128M)
#define SA1100_ROM_BANK2_BASE                    (SA1100_ROM_BANK1_BASE + SZ_128M)
#define SA1100_ROM_BANK3_BASE                    (SA1100_ROM_BANK2_BASE + SZ_128M)

#define SA1100_RAM_BANK0_BASE                    (0xC0000000)
#define SA1100_RAM_BANK1_BASE                    (SA1100_RAM_BANK0_BASE + SZ_128M)
#define SA1100_RAM_BANK2_BASE                    (SA1100_RAM_BANK1_BASE + SZ_128M)
#define SA1100_RAM_BANK3_BASE                    (SA1100_RAM_BANK2_BASE + SZ_128M)

#define SA1100_ZEROS_BANK_BASE                   (SA1100_RAM_BANK3_BASE + SZ_128M)

/*
 * SA1100 Register Definitions
 */
#define SA1100_REGISTER_BASE                     0x80000000
#define SA1100_REGISTER(x)                       REG32_PTR(SA1100_REGISTER_BASE + (x))

/*
 * SA-1100 Cache and MMU Definitions
 */
#define SA1100_ICACHE_SIZE                       SZ_16K
#define SA1100_DCACHE_SIZE                       SZ_8K
#define SA1100_ICACHE_LINESIZE_BYTES             32
#define SA1100_DCACHE_LINESIZE_BYTES             32
#define SA1100_ICACHE_LINESIZE_WORDS             8
#define SA1100_DCACHE_LINESIZE_WORDS             8
#define SA1100_ICACHE_WAYS                       32
#define SA1100_DCACHE_WAYS                       32
#define SA1100_ICACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(SA1100_ICACHE_LINESIZE_BYTES - 1))
#define SA1100_DCACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(SA1100_DCACHE_LINESIZE_BYTES - 1))

/*
 * SA-1100 Coprocessor 15 Extensions Register Definitions
 */
#ifdef __ASSEMBLER__
#  define SA1100_READ_PROCESS_ID_REGISTER        c13
#  define SA1100_WRITE_PROCESS_ID_REGISTER       c13
#  define SA1100_READ_BREAKPOINT_REGISTER        c14
#  define SA1100_WRITE_BREAKPOINT_REGISTER       c14
#  define SA1100_TEST_CLOCK_AND_IDLE_REGISTER    c15
#else /* __ASSEMBLER__ */
#  define SA1100_READ_PROCESS_ID_REGISTER        "c13"
#  define SA1100_WRITE_PROCESS_ID_REGISTER       "c13"
#  define SA1100_READ_BREAKPOINT_REGISTER        "c14"
#  define SA1100_WRITE_BREAKPOINT_REGISTER       "c14"
#  define SA1100_TEST_CLOCK_AND_IDLE_REGISTER    "c15"
#endif /* __ASSEMBLER__ */

/*
 * SA-1100 Process ID Virtual Address Mapping Definitions
 */
#ifdef __ASSEMBLER__
#  define SA1100_ACCESS_PROC_ID_REGISTER_OPCODE  0x0
#  define SA1100_ACCESS_PROC_ID_REGISTER_RM      c0
#else /* __ASSEMBLER__ */
#  define SA1100_ACCESS_PROC_ID_REGISTER_OPCODE  "0x0"
#  define SA1100_ACCESS_PROC_ID_REGISTER_RM      "c0"
#endif /* __ASSEMBLER__ */

#define SA1100_PROCESS_ID_PID_MASK               0x7E000000

/*
 * SA-1100 Debug Support Definitions
 */
#ifdef __ASSEMBLER__
#  define SA1100_ACCESS_DBAR_OPCODE              0x0
#  define SA1100_ACCESS_DBAR_RM                  c0
#  define SA1100_ACCESS_DBVR_OPCODE              0x0
#  define SA1100_ACCESS_DBVR_RM                  c1
#  define SA1100_ACCESS_DBMR_OPCODE              0x0
#  define SA1100_ACCESS_DBMR_RM                  c2
#  define SA1100_LOAD_DBCR_OPCODE                0x0
#  define SA1100_LOAD_DBCR_RM                    c3
#else /* __ASSEMBLER__ */
#  define SA1100_ACCESS_DBAR_OPCODE              "0x0"
#  define SA1100_ACCESS_DBAR_RM                  "c0"
#  define SA1100_ACCESS_DBVR_OPCODE              "0x0"
#  define SA1100_ACCESS_DBVR_RM                  "c1"
#  define SA1100_ACCESS_DBMR_OPCODE              "0x0"
#  define SA1100_ACCESS_DBMR_RM                  "c2"
#  define SA1100_LOAD_DBCR_OPCODE                "0x0"
#  define SA1100_LOAD_DBCR_RM                    "c3"
#endif /* __ASSEMBLER__ */

#define SA1100_DBCR_LOAD_WATCH_DISABLED          0x00000000
#define SA1100_DBCR_LOAD_WATCH_ENABLED           0x00000001
#define SA1100_DBCR_LOAD_WATCH_MASK              0x00000001
#define SA1100_DBCR_STORE_ADDRESS_WATCH_DISABLED 0x00000000
#define SA1100_DBCR_STORE_ADDRESS_WATCH_ENABLED  0x00000002
#define SA1100_DBCR_STORE_ADDRESS_WATCH_MASK     0x00000002
#define SA1100_DBCR_STORE_DATA_WATCH_DISABLED    0x00000000
#define SA1100_DBCR_STORE_DATA_WATCH_ENABLED     0x00000004
#define SA1100_DBCR_STORE_DATA_WATCH_MASK        0x00000004

#define SA1100_IBCR_INSTRUCTION_ADDRESS_MASK     0xFFFFFFFC
#define SA1100_IBCR_BREAKPOINT_DISABLED          0x00000000
#define SA1100_IBCR_BREAKPOINT_ENABLED           0x00000001
#define SA1100_IBCR_BREAKPOINT_ENABLE_MASK       0x00000001

/*
 * SA-1100 Test, Clock and Idle Control Definition
 */
#ifdef __ASSEMBLER__
#  define SA1100_ICACHE_ODD_WORD_LOADING_OPCODE  0x1
#  define SA1100_ICACHE_ODD_WORD_LOADING_RM      c1
#  define SA1100_ICACHE_EVEN_WORD_LOADING_OPCODE 0x1
#  define SA1100_ICACHE_EVEN_WORD_LOADING_RM     c2
#  define SA1100_ICACHE_CLEAR_LFSR_OPCODE        0x1
#  define SA1100_ICACHE_CLEAR_LFSR_RM            c4
#  define SA1100_MOVE_LFSR_TO_R14_ABORT_OPCODE   0x1
#  define SA1100_MOVE_LFSR_TO_R14_ABORT_RM       c8
#  define SA1100_ENABLE_CLOCK_SWITCHING_OPCODE   0x2
#  define SA1100_ENABLE_CLOCK_SWITCHING_RM       c1
#  define SA1100_DISABLE_CLOCK_SWITCHING_OPCODE  0x2
#  define SA1100_DISABLE_CLOCK_SWITCHING_RM      c2
#  define SA1100_WAIT_FOR_INTERRUPT_OPCODE       0x2
#  define SA1100_WAIT_FOR_INTERRUPT_RM           c8
#else /* __ASSEMBLER__ */
#  define SA1100_ICACHE_ODD_WORD_LOADING_OPCODE  "0x1"
#  define SA1100_ICACHE_ODD_WORD_LOADING_RM      "c1"
#  define SA1100_ICACHE_EVEN_WORD_LOADING_OPCODE "0x1"
#  define SA1100_ICACHE_EVEN_WORD_LOADING_RM     "c2"
#  define SA1100_ICACHE_CLEAR_LFSR_OPCODE        "0x1"
#  define SA1100_ICACHE_CLEAR_LFSR_RM            "c4"
#  define SA1100_MOVE_LFSR_TO_R14_ABORT_OPCODE   "0x1"
#  define SA1100_MOVE_LFSR_TO_R14_ABORT_RM       "c8"
#  define SA1100_ENABLE_CLOCK_SWITCHING_OPCODE   "0x2"
#  define SA1100_ENABLE_CLOCK_SWITCHING_RM       "c1"
#  define SA1100_DISABLE_CLOCK_SWITCHING_OPCODE  "0x2"
#  define SA1100_DISABLE_CLOCK_SWITCHING_RM      "c2"
#  define SA1100_WAIT_FOR_INTERRUPT_OPCODE       "0x2"
#  define SA1100_WAIT_FOR_INTERRUPT_RM           "c8"
#endif /* __ASSEMBLER__ */

/*
 * SA-1100 GPIO Register Definitions
 */
#define SA1100_GPIO_PIN_0                        BIT0
#define SA1100_GPIO_PIN_1                        BIT1
#define SA1100_GPIO_PIN_2                        BIT2
#define SA1100_GPIO_PIN_3                        BIT3
#define SA1100_GPIO_PIN_4                        BIT4
#define SA1100_GPIO_PIN_5                        BIT5
#define SA1100_GPIO_PIN_6                        BIT6
#define SA1100_GPIO_PIN_7                        BIT7
#define SA1100_GPIO_PIN_8                        BIT8
#define SA1100_GPIO_PIN_9                        BIT9
#define SA1100_GPIO_PIN_10                       BIT10
#define SA1100_GPIO_PIN_11                       BIT11
#define SA1100_GPIO_PIN_12                       BIT12
#define SA1100_GPIO_PIN_13                       BIT13
#define SA1100_GPIO_PIN_14                       BIT14
#define SA1100_GPIO_PIN_15                       BIT15
#define SA1100_GPIO_PIN_16                       BIT16
#define SA1100_GPIO_PIN_17                       BIT17
#define SA1100_GPIO_PIN_18                       BIT18
#define SA1100_GPIO_PIN_19                       BIT19
#define SA1100_GPIO_PIN_20                       BIT20
#define SA1100_GPIO_PIN_21                       BIT21
#define SA1100_GPIO_PIN_22                       BIT22
#define SA1100_GPIO_PIN_23                       BIT23
#define SA1100_GPIO_PIN_24                       BIT24
#define SA1100_GPIO_PIN_25                       BIT25
#define SA1100_GPIO_PIN_26                       BIT26
#define SA1100_GPIO_PIN_27                       BIT27

#define SA1100_GPIO_PIN_LEVEL_REGISTER_o         0x10040000
#define SA1100_GPIO_PIN_DIR_REGISTER_o           0x10040004
#define SA1100_GPIO_PIN_OUTPUT_SET_REGISTER_o    0x10040008
#define SA1100_GPIO_PIN_OUTPUT_CLEAR_REGISTER_o  0x1004000C
#define SA1100_GPIO_RISING_EDGE_DETECT_o         0x10040010
#define SA1100_GPIO_FALLING_EDGE_DETECT_o        0x10040014
#define SA1100_GPIO_EDGE_DETECT_STATUS_o         0x10040018
#define SA1100_GPIO_ALTERNATE_FUNCTION_o         0x1004001C

#define SA1100_GPIO_PIN_LEVEL_REGISTER           SA1100_REGISTER(SA1100_GPIO_PIN_LEVEL_REGISTER_o)
#define SA1100_GPIO_PIN_DIR_REGISTER             SA1100_REGISTER(SA1100_GPIO_PIN_DIR_REGISTER_o)
#define SA1100_GPIO_PIN_OUTPUT_SET_REGISTER      SA1100_REGISTER(SA1100_GPIO_PIN_OUTPUT_SET_REGISTER_o)
#define SA1100_GPIO_PIN_OUTPUT_CLEAR_REGISTER    SA1100_REGISTER(SA1100_GPIO_PIN_OUTPUT_CLEAR_REGISTER_o)
#define SA1100_GPIO_RISING_EDGE_DETECT           SA1100_REGISTER(SA1100_GPIO_RISING_EDGE_DETECT_o)
#define SA1100_GPIO_FALLING_EDGE_DETECT          SA1100_REGISTER(SA1100_GPIO_FALLING_EDGE_DETECT_o)
#define SA1100_GPIO_EDGE_DETECT_STATUS           SA1100_REGISTER(SA1100_GPIO_EDGE_DETECT_STATUS_o)
#define SA1100_GPIO_ALTERNATE_FUNCTION           SA1100_REGISTER(SA1100_GPIO_ALTERNATE_FUNCTION_o)

/*
 * SA1100 IRQ Controller IRQ Numbers
 */
#define SA1100_IRQ_MIN                           0
                                                 
#define SA1100_IRQ_GPIO_0_EDGE_DETECT            0
#define SA1100_IRQ_GPIO_1_EDGE_DETECT            1
#define SA1100_IRQ_GPIO_2_EDGE_DETECT            2
#define SA1100_IRQ_GPIO_3_EDGE_DETECT            3
#define SA1100_IRQ_GPIO_4_EDGE_DETECT            4
#define SA1100_IRQ_GPIO_5_EDGE_DETECT            5
#define SA1100_IRQ_GPIO_6_EDGE_DETECT            6
#define SA1100_IRQ_GPIO_7_EDGE_DETECT            7
#define SA1100_IRQ_GPIO_8_EDGE_DETECT            8
#define SA1100_IRQ_GPIO_9_EDGE_DETECT            0
#define SA1100_IRQ_GPIO_10_EDGE_DETECT           10
#define SA1100_IRQ_GPIO_ANY_EDGE_DETECT          11
#define SA1100_IRQ_LCD_CONT_SERVICE_REQUEST      12
#define SA1100_IRQ_USB_SERVICE_REQUEST           13
#define SA1100_IRQ_SDLC_SERVICE_REQUEST          14
#define SA1100_IRQ_UART1_SERVICE_REQUEST         15
#define SA1100_IRQ_ICP_SERVICE_REQUEST           16
#define SA1100_IRQ_UART3_SERVICE_REQUEST         17
#define SA1100_IRQ_MCP_SERVICE_REQUEST           18
#define SA1100_IRQ_SSP_SERVICE_REQUEST           19
#define SA1100_IRQ_CHANNEL_0_SERVICE_REQUEST     20
#define SA1100_IRQ_CHANNEL_1_SERVICE_REQUEST     21
#define SA1100_IRQ_CHANNEL_2_SERVICE_REQUEST     22
#define SA1100_IRQ_CHANNEL_3_SERVICE_REQUEST     23
#define SA1100_IRQ_CHANNEL_4_SERVICE_REQUEST     24
#define SA1100_IRQ_CHANNEL_5_SERVICE_REQUEST     25
#define SA1100_IRQ_OS_TIMER_MATCH_REG_0          26
#define SA1100_IRQ_OS_TIMER_MATCH_REG_1          27
#define SA1100_IRQ_OS_TIMER_MATCH_REG_2          28
#define SA1100_IRQ_OS_TIMER_MATCH_REG_3          29
#define SA1100_IRQ_ONE_HZ_CLOCK_TIC              30
#define SA1100_IRQ_RTC_EQUALS_ALARM              31
                                                 
#define SA1100_IRQ_MAX                           31
#define NUM_SA1100_INTERRUPTS                    SA1100_IRQ_MAX - SA1100_IRQ_MIN + 1
#define SA1100_IRQ_INTSRC_MASK(irq_nr)           (1 << (irq_nr))

/*
 * SA1100 IRQ Controller Register Definitions.
 */
#define SA1100_ICIP_o                            0x10050000
#define SA1100_ICMR_o                            0x10050004
#define SA1100_ICLR_o                            0x10050008
#define SA1100_ICCR_o                            0x1005000C
#define SA1100_ICFP_o                            0x10050010
#define SA1100_ICPR_o                            0x10050020

#define SA1100_ICIP                              SA1100_REGISTER(SA1100_ICIP_o)
#define SA1100_ICMR                              SA1100_REGISTER(SA1100_ICMR_o)
#define SA1100_ICLR                              SA1100_REGISTER(SA1100_ICLR_o)
#define SA1100_ICCR                              SA1100_REGISTER(SA1100_ICCR_o)
#define SA1100_ICFP                              SA1100_REGISTER(SA1100_ICFP_o)
#define SA1100_ICPR                              SA1100_REGISTER(SA1100_ICPR_o)

/*
 * SA1100 IRQ Controller Control Register Bit Fields.
 */
#define SA1100_ICCR_DISABLE_IDLE_MASK_ALL        0x0
#define SA1100_ICCR_DISABLE_IDLE_MASK_ENABLED    0x1


/*
 * SA1100 Peripheral Port Controller Register Definitions
 */
#define SA1100_PPC_PIN_DIRECTION_o               0x10060000
#define SA1100_PPC_PIN_STATE_o                   0x10060004
#define SA1100_PPC_PIN_ASSIGNMENT_o              0x10060008
#define SA1100_PPC_PIN_SLEEP_MODE_DIR_o          0x1006000C
#define SA1100_PPC_PIN_FLAG_o                    0x10060010

#define SA1100_PPC_PIN_DIRECTION                 SA1100_REGISTER(SA1100_PPC_PIN_DIRECTION_o)
#define SA1100_PPC_PIN_STATE                     SA1100_REGISTER(SA1100_PPC_PIN_STATE_o)
#define SA1100_PPC_PIN_ASSIGNMENT                SA1100_REGISTER(SA1100_PPC_PIN_ASSIGNMENT_o)
#define SA1100_PPC_PIN_SLEEP_MODE_DIR            SA1100_REGISTER(SA1100_PPC_PIN_SLEEP_MODE_DIR_o)
#define SA1100_PPC_PIN_FLAG                      SA1100_REGISTER(SA1100_PPC_PIN_FLAG_o)

/*
 * SA1100 PPC Bit Field Definitions
 */
#define SA1100_PPC_LCD_PIN_0_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_0_DIR_OUTPUT          0x00000001
#define SA1100_PPC_LCD_PIN_0_DIR_MASK            0x00000001
#define SA1100_PPC_LCD_PIN_1_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_1_DIR_OUTPUT          0x00000002
#define SA1100_PPC_LCD_PIN_1_DIR_MASK            0x00000002
#define SA1100_PPC_LCD_PIN_2_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_2_DIR_OUTPUT          0x00000004
#define SA1100_PPC_LCD_PIN_2_DIR_MASK            0x00000004
#define SA1100_PPC_LCD_PIN_3_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_3_DIR_OUTPUT          0x00000008
#define SA1100_PPC_LCD_PIN_3_DIR_MASK            0x00000008
#define SA1100_PPC_LCD_PIN_4_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_4_DIR_OUTPUT          0x00000010
#define SA1100_PPC_LCD_PIN_4_DIR_MASK            0x00000010
#define SA1100_PPC_LCD_PIN_5_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_5_DIR_OUTPUT          0x00000020
#define SA1100_PPC_LCD_PIN_5_DIR_MASK            0x00000020
#define SA1100_PPC_LCD_PIN_6_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_6_DIR_OUTPUT          0x00000040
#define SA1100_PPC_LCD_PIN_6_DIR_MASK            0x00000040
#define SA1100_PPC_LCD_PIN_7_DIR_INPUT           0x00000000
#define SA1100_PPC_LCD_PIN_7_DIR_OUTPUT          0x00000080
#define SA1100_PPC_LCD_PIN_7_DIR_MASK            0x00000080
#define SA1100_PPC_LCD_PIXCLK_DIR_INPUT          0x00000000
#define SA1100_PPC_LCD_PIXCLK_DIR_OUTPUT         0x00000100
#define SA1100_PPC_LCD_PIXCLK_DIR_MASK           0x00000100
#define SA1100_PPC_LCD_LINECLK_DIR_INPUT         0x00000000
#define SA1100_PPC_LCD_LINECLK_DIR_OUTPUT        0x00000200
#define SA1100_PPC_LCD_LINECLK_DIR_MASK          0x00000200
#define SA1100_PPC_LCD_FRAMECLK_DIR_INPUT        0x00000000
#define SA1100_PPC_LCD_FRAMECLK_DIR_OUTPUT       0x00000400
#define SA1100_PPC_LCD_FRAMECLK_DIR_MASK         0x00000400
#define SA1100_PPC_LCD_AC_BIAS_DIR_INPUT         0x00000000
#define SA1100_PPC_LCD_AC_BIAS_DIR_OUTPUT        0x00000800
#define SA1100_PPC_LCD_AC_BIAS_DIR_MASK          0x00000800
#define SA1100_PPC_SERIAL_PORT_1_TX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_1_TX_DIR_OUTPUT   0x00001000
#define SA1100_PPC_SERIAL_PORT_1_TX_DIR_MASK     0x00001000
#define SA1100_PPC_SERIAL_PORT_1_RX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_1_RX_DIR_OUTPUT   0x00002000
#define SA1100_PPC_SERIAL_PORT_1_RX_DIR_MASK     0x00002000
#define SA1100_PPC_SERIAL_PORT_2_TX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_2_TX_DIR_OUTPUT   0x00004000
#define SA1100_PPC_SERIAL_PORT_2_TX_DIR_MASK     0x00004000
#define SA1100_PPC_SERIAL_PORT_2_RX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_2_RX_DIR_OUTPUT   0x00008000
#define SA1100_PPC_SERIAL_PORT_2_RX_DIR_MASK     0x00008000
#define SA1100_PPC_SERIAL_PORT_3_TX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_3_TX_DIR_OUTPUT   0x00010000
#define SA1100_PPC_SERIAL_PORT_3_TX_DIR_MASK     0x00010000
#define SA1100_PPC_SERIAL_PORT_3_RX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_3_RX_DIR_OUTPUT   0x00020000
#define SA1100_PPC_SERIAL_PORT_3_RX_DIR_MASK     0x00020000
#define SA1100_PPC_SERIAL_PORT_4_TX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_4_TX_DIR_OUTPUT   0x00040000
#define SA1100_PPC_SERIAL_PORT_4_TX_DIR_MASK     0x00040000
#define SA1100_PPC_SERIAL_PORT_4_RX_DIR_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_4_RX_DIR_OUTPUT   0x00080000
#define SA1100_PPC_SERIAL_PORT_4_RX_DIR_MASK     0x00080000
#define SA1100_PPC_SERIAL_PORT_4_SERCLK_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_4_SERCLK_OUTPUT   0x00100000
#define SA1100_PPC_SERIAL_PORT_4_SERCLK_MASK     0x00100000
#define SA1100_PPC_SERIAL_PORT_4_SERFRM_INPUT    0x00000000
#define SA1100_PPC_SERIAL_PORT_4_SERFRM_OUTPUT   0x00200000
#define SA1100_PPC_SERIAL_PORT_4_SERFRM_MASK     0x00200000

#define SA1100_PPC_UART_PIN_NOT_REASSIGNED       0x00000000
#define SA1100_PPC_UART_PIN_REASSIGNED           0x00001000
#define SA1100_PPC_UART_PIN_REASSIGNMENT_MASK    0x00001000
#define SA1100_PPC_SSP_PIN_NOT_REASSIGNED        0x00000000
#define SA1100_PPC_SSP_PIN_REASSIGNED            0x00040000
#define SA1100_PPC_SSP_PIN_REASSIGNMENT_MASK     0x00040000


/*
 * SA1100 UART 1 Registers
 */
#define SA1100_UART_CONTROL_0_o                  0x00000000
#define SA1100_UART_CONTROL_1_o                  0x00000004
#define SA1100_UART_CONTROL_2_o                  0x00000008
#define SA1100_UART_CONTROL_3_o                  0x0000000C
#define SA1100_UART_DATA_o                       0x00000014
#define SA1100_UART_STATUS_0_o                   0x0000001C
#define SA1100_UART_STATUS_1_o                   0x00000020

#define SA1100_UART_1_BASE_o                     0x00010000
#define SA1100_UART_1_REGISTER(x)                SA1100_REGISTER(x + SA1100_UART_1_BASE_o)
#define SA1100_UART_1_CONTROL_0                  SA1100_UART_1_REGISTER(SA1100_UART_CONTROL_0_o)
#define SA1100_UART_1_CONTROL_1                  SA1100_UART_1_REGISTER(SA1100_UART_CONTROL_1_o)
#define SA1100_UART_1_CONTROL_2                  SA1100_UART_1_REGISTER(SA1100_UART_CONTROL_2_o)
#define SA1100_UART_1_CONTROL_3                  SA1100_UART_1_REGISTER(SA1100_UART_CONTROL_3_o)
#define SA1100_UART_1_DATA                       SA1100_UART_1_REGISTER(SA1100_UART_DATA_o)
#define SA1100_UART_1_STATUS_0                   SA1100_UART_1_REGISTER(SA1100_UART_STATUS_0_o)
#define SA1100_UART_1_STATUS_1                   SA1100_UART_1_REGISTER(SA1100_UART_STATUS_1_o)

/*
 * SA1100 UART 3 Registers
 */
#define SA1100_UART_3_BASE_o                     0x00050000
#define SA1100_UART_3_REGISTER(x)                SA1100_REGISTER(x + SA1100_UART_3_BASE_o)
#define SA1100_UART_CONTROL_0                    SA1100_UART_3_REGISTER(SA1100_UART_CONTROL_0_o)
#define SA1100_UART_CONTROL_1                    SA1100_UART_3_REGISTER(SA1100_UART_CONTROL_1_o)
#define SA1100_UART_CONTROL_2                    SA1100_UART_3_REGISTER(SA1100_UART_CONTROL_2_o)
#define SA1100_UART_CONTROL_3                    SA1100_UART_3_REGISTER(SA1100_UART_CONTROL_3_o)
#define SA1100_UART_DATA                         SA1100_UART_3_REGISTER(SA1100_UART_DATA_o)
#define SA1100_UART_STATUS_0                     SA1100_UART_3_REGISTER(SA1100_UART_STATUS_0_o)
#define SA1100_UART_STATUS_1                     SA1100_UART_3_REGISTER(SA1100_UART_STATUS_1_o)

/*
 * SA1100 UART Control Register 0 Bit Fields.
 */
#define SA1100_UART_PARITY_DISABLED              0x00
#define SA1100_UART_PARITY_ENABLED               0x01
#define SA1100_UART_PARITY_ENABLE_MASK           0x01
#define SA1100_UART_PARITY_ODD                   0x00
#define SA1100_UART_PARITY_EVEN                  0x02
#define SA1100_UART_PARITY_MODE_MASK             0x02
#define SA1100_UART_STOP_BITS_1                  0x00
#define SA1100_UART_STOP_BITS_2                  0x04
#define SA1100_UART_STOP_BITS_MASK               0x04
#define SA1100_UART_DATA_BITS_7                  0x00
#define SA1100_UART_DATA_BITS_8                  0x08
#define SA1100_UART_DATA_BITS_MASK               0x08
#define SA1100_UART_SAMPLE_CLOCK_DISABLED        0x00
#define SA1100_UART_SAMPLE_CLOCK_ENABLED         0x10
#define SA1100_UART_SAMPLE_CLOCK_ENABLE_MASK     0x10
#define SA1100_UART_RX_RISING_EDGE_SELECT        0x00
#define SA1100_UART_RX_FALLING_EDGE_SELECT       0x20
#define SA1100_UART_RX_EDGE_SELECT_MASK          0x20
#define SA1100_UART_TX_RISING_EDGE_SELECT        0x00
#define SA1100_UART_TX_FALLING_EDGE_SELECT       0x40
#define SA1100_UART_TX_EDGE_SELECT_MASK          0x20

/*
 * SA-1100 UART Baud Control Register bit masks
 */
#define SA1100_UART_H_BAUD_RATE_DIVISOR_MASK     0x0000000F
#define SA1100_UART_L_BAUD_RATE_DIVISOR_MASK     0x000000FF

/*
 * SA-1100 UART Control Register 3 Bit Fields.
 */
#define SA1100_UART_RX_DISABLED                  0x00
#define SA1100_UART_RX_ENABLED                   0x01
#define SA1100_UART_RX_ENABLE_MASK               0x01
#define SA1100_UART_TX_DISABLED                  0x00
#define SA1100_UART_TX_ENABLED                   0x02
#define SA1100_UART_TX_ENABLE_MASK               0x02
#define SA1100_UART_BREAK_DISABLED               0x00
#define SA1100_UART_BREAK_ENABLED                0x04
#define SA1100_UART_BREAK_MASK                   0x04
#define SA1100_UART_RX_FIFO_INT_DISABLED         0x00
#define SA1100_UART_RX_FIFO_INT_ENABLED          0x08
#define SA1100_UART_RX_FIFO_INT_ENABLE_MASK      0x08
#define SA1100_UART_TX_FIFO_INT_DISABLED         0x00
#define SA1100_UART_TX_FIFO_INT_ENABLED          0x10
#define SA1100_UART_TX_FIFO_INT_ENABLE_MASK      0x10
#define SA1100_UART_NORMAL_OPERATION             0x00
#define SA1100_UART_LOOPBACK_MODE                0x20

/*
 * SA-1100 UART Data Register bit masks
 */
#define SA1100_UART_DATA_MASK                    0x000000FF

/*
 * SA-1100 UART Status Register 0 Bit Fields.
 */
#define SA1100_UART_TX_SERVICE_REQUEST           0x01
#define SA1100_UART_RX_SERVICE_REQUEST           0x02
#define SA1100_UART_RX_IDLE                      0x04
#define SA1100_UART_RX_BEGIN_OF_BREAK            0x08
#define SA1100_UART_RX_END_OF_BREAK              0x10
#define SA1100_UART_ERROR_IN_FIFO                0x20

/*
 * SA-1100 UART Status Register 1 Bit Fields.
 */
#define SA1100_UART_TX_BUSY                      0x01
#define SA1100_UART_RX_FIFO_NOT_EMPTY            0x02
#define SA1100_UART_TX_FIFO_NOT_FULL             0x04
#define SA1100_UART_PARITY_ERROR                 0x08
#define SA1100_UART_FRAMING_ERROR                0x10
#define SA1100_UART_RX_FIFO_OVERRUN              0x20

#define UART_BASE_0                              SA1100_UART_CONTROL_0
#define UART_BASE_1                              SA1100_UART_1_CONTROL_0


/*
 * SA-1100 MCP Registers
 */
#define SA1100_MCP_CONTROL_0_o                   0x00060000
#define SA1100_MCP_DATA_0_o                      0x00060008
#define SA1100_MCP_DATA_1_o                      0x0006000C
#define SA1100_MCP_DATA_2_o                      0x00060010
#define SA1100_MCP_STATUS_o                      0x00060018
#define SA1100_MCP_CONTROL_1_o                   0x00060030

#define SA1100_MCP_CONTROL_0                     SA1100_REGISTER(SA1100_MCP_CONTROL_0_o)
#define SA1100_MCP_DATA_0                        SA1100_REGISTER(SA1100_MCP_DATA_0_o)
#define SA1100_MCP_DATA_1                        SA1100_REGISTER(SA1100_MCP_DATA_1_o)
#define SA1100_MCP_DATA_2                        SA1100_REGISTER(SA1100_MCP_DATA_2_o)
#define SA1100_MCP_STATUS                        SA1100_REGISTER(SA1100_MCP_STATUS_o)
#define SA1100_MCP_CONTROL_1                     SA1100_REGISTER(SA1100_MCP_CONTROL_1_o)


/*
 * SA-1100 Memory Configuration Registers
 */
#define SA1100_DRAM_CONFIGURATION_o              0x20000000
#define SA1100_DRAM_CAS_0_o                      0x20000004
#define SA1100_DRAM_CAS_1_o                      0x20000008
#define SA1100_DRAM_CAS_2_o                      0x2000000C
#define SA1100_STATIC_CONTROL_0_o                0x20000010
#define SA1100_STATIC_CONTROL_1_o                0x20000014
#define SA1100_EXP_BUS_CONFIGURATION_o           0x20000018

#define SA1100_DRAM_CONFIGURATION                SA1100_REGISTER(SA1100_DRAM_CONFIGURATION_o)
#define SA1100_DRAM_CAS_0                        SA1100_REGISTER(SA1100_DRAM_CAS_0_o)
#define SA1100_DRAM_CAS_1                        SA1100_REGISTER(SA1100_DRAM_CAS_1_o)
#define SA1100_DRAM_CAS_2                        SA1100_REGISTER(SA1100_DRAM_CAS_2_o)
#define SA1100_STATIC_CONTROL_0                  SA1100_REGISTER(SA1100_STATIC_CONTROL_0_o)
#define SA1100_STATIC_CONTROL_1                  SA1100_REGISTER(SA1100_STATIC_CONTROL_1_o)
#define SA1100_EXP_BUS_CONFIGURATION             SA1100_REGISTER(SA1100_EXP_BUS_CONFIGURATION_o)

/*
 * SA-1100 DRAM Configuration Bit Field Definitions
 */
#define SA1100_DRAM_BANK_0_DISABLED              0x00000000
#define SA1100_DRAM_BANK_0_ENABLED               0x00000001
#define SA1100_DRAM_BANK_0_ENABLE_MASK           0x00000001
#define SA1100_DRAM_BANK_1_DISABLED              0x00000000
#define SA1100_DRAM_BANK_1_ENABLED               0x00000002
#define SA1100_DRAM_BANK_1_ENABLE_MASK           0x00000002
#define SA1100_DRAM_BANK_2_DISABLED              0x00000000
#define SA1100_DRAM_BANK_2_ENABLED               0x00000004
#define SA1100_DRAM_BANK_2_ENABLE_MASK           0x00000004
#define SA1100_DRAM_BANK_3_DISABLED              0x00000000
#define SA1100_DRAM_BANK_3_ENABLED               0x00000008
#define SA1100_DRAM_BANK_3_ENABLE_MASK           0x00000008
#define SA1100_DRAM_ROW_ADDRESS_BITS_9           0x00000000
#define SA1100_DRAM_ROW_ADDRESS_BITS_10          0x00000010
#define SA1100_DRAM_ROW_ADDRESS_BITS_11          0x00000020
#define SA1100_DRAM_ROW_ADDRESS_BITS_12          0x00000030
#define SA1100_DRAM_ROW_ADDRESS_BITS_MASK        0x00000030
#define SA1100_DRAM_CLOCK_CPU_CLOCK              0x00000000
#define SA1100_DRAM_CLOCK_CPU_CLOCK_DIV_2        0x00000040
#define SA1100_DRAM_CLOCK_CPU_CLOCK_MASK         0x00000040
#define SA1100_DRAM_RAS_PRECHARGE(x)             (((x) & 0xF) << 7)
#define SA1100_DRAM_CAS_BEFORE_RAS(x)            (((x) & 0xF) << 11)
#define SA1100_DATA_INPUT_LATCH_WITH_CAS         0x00000000
#define SA1100_DATA_INPUT_LATCH_CAS_PLUS_ONE     0x00008000
#define SA1100_DATA_INPUT_LATCH_CAS_PLUS_TWO     0x00010000
#define SA1100_DATA_INPUT_LATCH_CAS_PLUS_THREE   0x00018000
#define SA1100_DRAM_REFRESH_INTERVAL(x)          (((x) & 0x7FFF) << 17)

/*
 * SA-1100 Static Memory Control Register Bit Field Definitions
 */
#define SA1100_STATIC_ROM_TYPE_FLASH             0x00000000
#define SA1100_STATIC_ROM_TYPE_SRAM              0x00000001
#define SA1100_STATIC_ROM_TYPE_BURST_OF_4_ROM    0x00000002
#define SA1100_STATIC_ROM_TYPE_BURST_OF_8_ROM    0x00000003
#define SA1100_STATIC_ROM_TYPE_MASK              0x00000003
#define SA1100_STATIC_ROM_BUS_WIDTH_32_BITS      0x00000000
#define SA1100_STATIC_ROM_BUS_WIDTH_16_BITS      0x00000004
#define SA1100_STATIC_ROM_BUS_WIDTH_MASK         0x00000004
#define SA1100_STATIC_ROM_DELAY_FIRST_ACCESS(x)  (((x) & 0x1F) << 3)
#define SA1100_STATIC_ROM_DELAY_NEXT_ACCESS(x)   (((x) & 0x1F) << 8)
#define SA1100_STATIC_ROM_RECOVERY(x)            (((x) & 0x7) << 13)

#define SA1100_STATIC_ROM_BANK_0(x)              (((x) & 0xFFFF) <<  0)
#define SA1100_STATIC_ROM_BANK_1(x)              (((x) & 0xFFFF) << 16)
#define SA1100_STATIC_ROM_BANK_2(x)              (((x) & 0xFFFF) <<  0)
#define SA1100_STATIC_ROM_BANK_3(x)              (((x) & 0xFFFF) << 16)


/*
 * SA-1100 Power Manager Registers
 */
#define SA1100_PWR_MGR_CONTROL_o                 0x10020000
#define SA1100_PWR_MGR_SLEEP_STATUS_o            0x10020004
#define SA1100_PWR_MGR_SCRATCHPAD_o              0x10020008
#define SA1100_PWR_MGR_WAKEUP_ENABLE_o           0x1002000C
#define SA1100_PWR_MGR_GENERAL_CONFIG_o          0x10020010
#define SA1100_PWR_MGR_PLL_CONFIG_o              0x10020014
#define SA1100_PWR_MGR_GPIO_SLEEP_STATE_o        0x10020018
#define SA1100_PWR_MGR_OSCILLATOR_STATUS_o       0x1002001C

#define SA1100_PWR_MGR_CONTROL                   SA1100_REGISTER(SA1100_PWR_MGR_CONTROL_o)
#define SA1100_PWR_MGR_SLEEP_STATUS              SA1100_REGISTER(SA1100_PWR_MGR_SLEEP_STATUS_o)
#define SA1100_PWR_MGR_SCRATCHPAD                SA1100_REGISTER(SA1100_PWR_MGR_SCRATCHPAD_o)
#define SA1100_PWR_MGR_WAKEUP_ENABLE             SA1100_REGISTER(SA1100_PWR_MGR_WAKEUP_ENABLE_o)
#define SA1100_PWR_MGR_GENERAL_CONFIG            SA1100_REGISTER(SA1100_PWR_MGR_GENERAL_CONFIG_o)
#define SA1100_PWR_MGR_PLL_CONFIG                SA1100_REGISTER(SA1100_PWR_MGR_PLL_CONFIG_o)
#define SA1100_PWR_MGR_GPIO_SLEEP_STATE          SA1100_REGISTER(SA1100_PWR_MGR_GPIO_SLEEP_STATE_o)
#define SA1100_PWR_MGR_OSC_STATUS                SA1100_REGISTER(SA1100_POWER_MANAGER_OSC_STATUS_o)

/*
 * SA-1100 Control Register Bit Field Definitions
 */
#define SA1100_NO_FORCE_SLEEP_MODE               0x00000000
#define SA1100_FORCE_SLEEP_MODE                  0x00000001
#define SA1100_SLEEP_MODE_MASK                   0x00000001

/*
 * SA-1100 Power Management Configuration Register Bit Field Definitions
 */
#define SA1100_NO_STOP_OSC_DURING_SLEEP          0x00000000
#define SA1100_STOP_OSC_DURING_SLEEP             0x00000001
#define SA1100_OSC_DURING_SLEEP_MASK             0x00000001
#define SA1100_DRIVE_PCMCIA_DURING_SLEEP         0x00000000
#define SA1100_FLOAT_PCMCIA_DURING_SLEEP         0x00000002
#define SA1100_PCMCIA_DURING_SLEEP_MASK          0x00000002
#define SA1100_DRIVE_CHIPSEL_DURING_SLEEP        0x00000000
#define SA1100_FLOAT_CHIPSEL_DURING_SLEEP        0x00000004
#define SA1100_CHIPSEL_DURING_SLEEP_MASK         0x00000004
#define SA1100_WAIT_OSC_STABLE                   0x00000000
#define SA1100_FORCE_OSC_ENABLE_ON               0x00000008
#define SA1100_OSC_STABLE_MASK                   0x00000008

/*
 * SA-1100 PLL Configuration Register Bit Field Definitions
 */
#define SA1100_CLOCK_SPEED_59_0_MHz              0x00000000
#define SA1100_CLOCK_SPEED_73_7_MHz              0x00000001
#define SA1100_CLOCK_SPEED_88_5_MHz              0x00000002
#define SA1100_CLOCK_SPEED_103_2_MHz             0x00000003
#define SA1100_CLOCK_SPEED_118_0_MHz             0x00000004
#define SA1100_CLOCK_SPEED_132_7_MHz             0x00000005
#define SA1100_CLOCK_SPEED_147_5_MHz             0x00000006
#define SA1100_CLOCK_SPEED_162_2_MHz             0x00000007
#define SA1100_CLOCK_SPEED_176_9_MHz             0x00000008
#define SA1100_CLOCK_SPEED_191_7_MHz             0x00000009
#define SA1100_CLOCK_SPEED_206_4_MHz             0x0000000A
#define SA1100_CLOCK_SPEED_221_2_MHz             0x0000000B

/*
 * SA-1100 Power Manager Wakeup Register Bit Field Definitions
 */
#define SA1100_WAKEUP_ENABLE(x)                  ((x) & 0x8FFFFFFF)

/*
 * SA-1100 Power Manager Sleep Status Bit Field Definitions
 */
#define SA1100_SOFTWARE_SLEEP_STATUS             0x00000001
#define SA1100_BATTERY_FAULT_STATUS              0x00000002
#define SA1100_VDD_FAULT_STATUS                  0x00000004
#define SA1100_DRAM_CONTROL_HOLD                 0x00000008
#define SA1100_PERIPHERAL_CONTROL_HOLD           0x00000010

/*
 * SA-1100 Power Manager Oscillator Status Register Bit Field Definitions
 */
#define SA1100_OSCILLATOR_STATUS                 0x00000001

/*
 * SA-1110 GPCLK Register Definitions
 */
#define SA1110_GPCLK_CONTROL_0                   SA1100_REGISTER(0x00020060)
#define SA1110_GPCLK_CONTROL_1                   SA1100_REGISTER(0x0002006C)
#define SA1110_GPCLK_CONTROL_2                   SA1100_REGISTER(0x00020070)

/* GPCLK Control Register 0 */
#define SA1110_GPCLK_SUS_GPCLK   0
#define SA1110_GPCLK_SUS_UART    1
#define SA1110_GPCLK_SCE         2
#define SA1110_GPCLK_SCD_IN      0
#define SA1110_GPCLK_SCD_OUT     4

/*
 * SA-1100 Reset Controller Register Definitions
 */
#define SA1100_RESET_SOFTWARE_RESET_o            0x10030000
#define SA1100_RESET_STATUS_o                    0x10030004

#define SA1100_RESET_SOFTWARE_RESET              SA1100_REGISTER(SA1100_RESET_SOFTWARE_RESET_o)
#define SA1100_RESET_STATUS                      SA1100_REGISTER(SA1100_RESET_STATUS_o)

/*
 * SA-1100 Reset Controller Bit Field Definitions
 */
#define SA1100_INVOKE_SOFTWARE_RESET             0x1

#define SA1100_HARDWARE_RESET                    0x1
#define SA1100_SOFTWARE_RESET                    0x2
#define SA1100_WATCHDOG_RESET                    0x4
#define SA1100_SLEEP_MODE_RESET                  0x8


#define PlatformRegBase         0x10000000
/* platform registers - offsets from PlatformRegBase */
#define PLAT_WHOAMI                  (0x0)
#define PLAT_LED                     (0x10)
#define PLAT_SWITCHES                (0x20)
#define PLAT_KEYCOL                  (0x80)
#define PLAT_KEYROW                  (0x90)
#define PLAT_NCR                     (0xA0)
#define PLAT_MDM0                    (0xB0)
#define PLAT_MDM1                    (0xB4)
#define PLAT_AUDIO                   (0xC0)

/* Tick counter registers */
#define SCM_BASE                     (0x90000000)    // base addr of scm module
#define OSCR                         (0x10)          // offset from scm_base
#define OSSR                         (0x14)          // offset from scm_base
#define RTC_OFFSET                   (0x10000)       // offset from scm_base
#define RCNR                         (0x4)           // offset from rtc_offset
#define RTTR                         (0x8)           // offset from rtc_offset
#define RTSR                         (0x10)          // offset from rtc_offset

#define GPIOREGBASE                  0x90040000
#define GPIO_OFFSET                  0x40000      ; offset from scm_base
#define GPLR                         0x00
#define GPDR                         0x04
#define GPSR                         0x08
#define GPCR                         0x0C
#define GRER                         0x10
#define GFER                         0x14
#define GEDR                         0x18
#define GAFR                         0x1C

#define GPDR_SETTINGS                0x080337FC


#ifndef __ASSEMBLER__
static inline void __board_reset(void)
{
    *SA1100_RESET_SOFTWARE_RESET |= SA1100_INVOKE_SOFTWARE_RESET;
    return;
}
#endif /* __ASSEMBLER__ */

#endif /* __SA1100_H__ */
