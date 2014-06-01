/*
 * armpid.h -- Header for ARM(R) Pid Eval Board
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
#ifndef __ARMPID_H__
#define __ARMPID_H__ 1

#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * ROM Definitions
 */
#define ROM_ACTUAL_BASE     0x04000000
#define ROM_TOTAL_SIZE      SZ_64M
#define ROM_VIRTUAL_BASE    0x04000000

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     0x00000000
#define RAM_TOTAL_SIZE      SZ_512K
#define RAM_VIRTUAL_BASE    0x00000000

#ifdef __ASSEMBLER__
#define REG_PTR(a) (a)
#else
#define REG_PTR(a) ((volatile int *)(a))
#endif

/*
 * General ARM(R) Pid register groups
 */
#define ARMPID_INT_CONT_BASE                    0x0A000000
#define ARMPID_CNT_TMR_BASE                     0x0A800000
#define ARMPID_REMAP_BASE                       0x0B000000
#define ARMPID_EXPANSION_BASE                   0x0B800000
#define ARMPID_NISA_IO_BASE                     0x0C000000
#define ARMPID_NISA_MEM_BASE                    0x0E000000

/*
 * IRQ Controller Registers
 */
#define ARMPID_IRQCONT_IRQSTATUS                REG_PTR(ARMPID_INT_CONT_BASE + 0x000)
#define ARMPID_IRQCONT_IRQRAWSTATUS             REG_PTR(ARMPID_INT_CONT_BASE + 0x004)
#define ARMPID_IRQCONT_IRQENABLE                REG_PTR(ARMPID_INT_CONT_BASE + 0x008)
#define ARMPID_IRQCONT_IRQENABLESET             REG_PTR(ARMPID_INT_CONT_BASE + 0x008)
#define ARMPID_IRQCONT_IRQENABLECLEAR           REG_PTR(ARMPID_INT_CONT_BASE + 0x00C)
#define ARMPID_IRQCONT_IRQSOFT                  REG_PTR(ARMPID_INT_CONT_BASE + 0x010)
#define ARMPID_IRQCONT_FIQSTATUS                REG_PTR(ARMPID_INT_CONT_BASE + 0x100)
#define ARMPID_IRQCONT_FIQRAWSTATUS             REG_PTR(ARMPID_INT_CONT_BASE + 0x104)
#define ARMPID_IRQCONT_FIQENABLE                REG_PTR(ARMPID_INT_CONT_BASE + 0x108)
#define ARMPID_IRQCONT_FIQENABLESET             REG_PTR(ARMPID_INT_CONT_BASE + 0x108)
#define ARMPID_IRQCONT_FIQENABLECLEAR           REG_PTR(ARMPID_INT_CONT_BASE + 0x10C)
#define ARMPID_IRQCONT_FIQSOURCE                REG_PTR(ARMPID_INT_CONT_BASE + 0x114)

/*
 * IRQ Controller IRQ Numbers
 */
#define ARMPID_IRQ_MIN                          0
#define ARMPID_IRQ_FIQ                          0
#define ARMPID_IRQ_FIQ_EXTERNAL                 0
#define ARMPID_IRQ_PROGRAMMED_INTERRUPT         1
#define ARMPID_IRQ_DEBUG_CHANNEL_COMMS_RX       2
#define ARMPID_IRQ_DEBUG_CHANNEL_COMMS_TX       3
#define ARMPID_IRQ_TIMER_1                      4
#define ARMPID_IRQ_TIMER_2                      5
#define ARMPID_IRQ_PC_CARD_SLOT_A               6
#define ARMPID_IRQ_PC_CARD_SLOT_B               7
#define ARMPID_IRQ_SERIAL_PORT_A                8
#define ARMPID_IRQ_SERIAL_PORT_B                9
#define ARMPID_IRQ_PARALLEL_PORT                10
#define ARMPID_IRQ_ASB_EXPANSION_0              11
#define ARMPID_IRQ_ASB_EXPANSION_1              12
#define ARMPID_IRQ_APB_EXPANSION_0              13
#define ARMPID_IRQ_APB_EXPANSION_1              14
#define ARMPID_IRQ_APB_EXPANSION_2              15
#define ARMPID_IRQ_MAX                          15
#define NUM_ARMPID_INTERRUPTS                   ARMPID_IRQ_MAX - ARMPID_IRQ_MIN + 1
#define ARMPID_IRQ_INTSRC_MASK(irq_nr)          (1 << (irq_nr))

/*
 * Counter/Timer Registers
 */
#define ARMPID_CNT_TMR_TIMER1_LOAD         REG_PTR(ARMPID_CNT_TMR_BASE + 0x00)
#define ARMPID_CNT_TMR_TIMER1_VALUE        REG_PTR(ARMPID_CNT_TMR_BASE + 0x04)
#define ARMPID_CNT_TMR_TIMER1_CONTROL      REG_PTR(ARMPID_CNT_TMR_BASE + 0x08)
#define ARMPID_CNT_TMR_TIMER1_CLEAR        REG_PTR(ARMPID_CNT_TMR_BASE + 0x0C)
#define ARMPID_CNT_TMR_TIMER2_LOAD         REG_PTR(ARMPID_CNT_TMR_BASE + 0x20)
#define ARMPID_CNT_TMR_TIMER2_VALUE        REG_PTR(ARMPID_CNT_TMR_BASE + 0x24)
#define ARMPID_CNT_TMR_TIMER2_CONTROL      REG_PTR(ARMPID_CNT_TMR_BASE + 0x28)
#define ARMPID_CNT_TMR_TIMER2_CLEAR        REG_PTR(ARMPID_CNT_TMR_BASE + 0x2C)

/*
 * Counter/Timer Control Register Bit fields
 */
#define ARMPID_CNT_TMR_CONTROL_DISABLED    0x00000000
#define ARMPID_CNT_TMR_CONTROL_ENABLED     0x00000080
#define ARMPID_CNT_TMR_CONTROL_FREE_RUN    0x00000000
#define ARMPID_CNT_TMR_CONTROL_PERIODIC    0x00000040
#define ARMPID_CNT_TMR_CONTROL_CLK_DIV_1   0x00000000
#define ARMPID_CNT_TMR_CONTROL_CLK_DIV_16  0x00000004
#define ARMPID_CNT_TMR_CONTROL_CLK_DIV_256 0x00000008

/*
 * Reset/Pause Registers
 */
#define ARMPID_REMAP_PAUSE                 REG_PTR(ARMPID_REMAP_BASE + 0x00)
#define ARMPID_REMAP_ID                    REG_PTR(ARMPID_REMAP_BASE + 0x10)
#define ARMPID_REMAP_CLEAR_RST_MAP         REG_PTR(ARMPID_REMAP_BASE + 0x20)
#define ARMPID_REMAP_RST_STATUS            REG_PTR(ARMPID_REMAP_BASE + 0x30)
#define ARMPID_REMAP_RST_STATUS_SET        REG_PTR(ARMPID_REMAP_BASE + 0x30)
#define ARMPID_REMAP_RST_STATUS_CLR        REG_PTR(ARMPID_REMAP_BASE + 0x34)

/*
 * Remap/Pause Bit fields
 */
#define ARMPID_REMAP_ID_NO_FURTHER_INFO    0x00000000
#define ARMPID_REMAP_ID_FURTHER_INFO       0x00000001
#define ARMPID_REMAP_NO_POWER_ON_RESET     0x00000000
#define ARMPID_REMAP_POWER_ON_RESET        0x00000001

/*
 * NISA Bus IO devices
 */
#define ARMPID_PCMCIA_BASE                 (ARMPID_NISA_IO_BASE + 0x7c0)
#define ARMPID_PCMCIA_MEM_BASE             (ARMPID_NISA_IO_BASE + 0x02000000)
#define ARMPID_SER_A_BASE                  (ARMPID_NISA_IO_BASE + 0x01800000)
#define ARMPID_SER_B_BASE                  (ARMPID_NISA_IO_BASE + 0x01800020)
#define ARMPID_PAR_BASE                    (ARMPID_NISA_IO_BASE + 0x01800040)

/*
 * Serial Port Register Offsets
 */
#define ARMPID_SER_RX_HOLDING_o            0x00
#define ARMPID_SER_TX_HOLDING_o            0x00
#define ARMPID_SER_INT_ENABLE_o            0x04
#define ARMPID_SER_INT_STATUS_o            0x08
#define ARMPID_SER_FIFO_CONTROL_o          0x08
#define ARMPID_SER_LINE_CONTROL_o          0x0c
#define ARMPID_SER_MODEM_CONTROL_o         0x10
#define ARMPID_SER_LINE_STATUS_o           0x14
#define ARMPID_SER_MODEM_STATUS_o          0x18
#define ARMPID_SER_SCRATCHPAD_o            0x1c
#define ARMPID_SER_DIVISOR_LSB_o           0x00
#define ARMPID_SER_DIVISOR_MSB_o           0x04

/*
 * Serial Port A Registers
 */
#define ARMPID_SER_A_RX_HOLDING            REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_RX_HOLDING_o)
#define ARMPID_SER_A_TX_HOLDING            REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_TX_HOLDING_o)
#define ARMPID_SER_A_INT_ENABLE            REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_INT_ENABLE_o)
#define ARMPID_SER_A_INT_STATUS            REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_INT_STATUS_o)
#define ARMPID_SER_A_FIFO_CONTROL          REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_FIFO_CONTROL_o)
#define ARMPID_SER_A_LINE_CONTROL          REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_LINE_CONTROL_o)
#define ARMPID_SER_A_MODEM_CONTROL         REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_MODEM_CONTROL_o)
#define ARMPID_SER_A_LINE_STATUS           REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_LINE_STATUS_o)
#define ARMPID_SER_A_MODEM_STATUS          REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_MODEM_STATUS_o)
#define ARMPID_SER_A_SCRATCHPAD            REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_SCRATCHPAD_o)
#define ARMPID_SER_A_DIVISOR_LSB           REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_DIVISOR_LSB_o)
#define ARMPID_SER_A_DIVISOR_MSB           REG_PTR(ARMPID_SER_A_BASE + ARMPID_SER_DIVISOR_MSB_o)

/*
 * Serial Port B Registers
 */
#define ARMPID_SER_B_RX_HOLDING            REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_RX_HOLDING_o)
#define ARMPID_SER_B_TX_HOLDING            REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_TX_HOLDING_o)
#define ARMPID_SER_B_INT_ENABLE            REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_INT_ENABLE_o)
#define ARMPID_SER_B_INT_STATUS            REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_INT_STATUS_o)
#define ARMPID_SER_B_FIFO_CONTROL          REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_FIFO_CONTROL_o)
#define ARMPID_SER_B_LINE_CONTROL          REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_LINE_CONTROL_o)
#define ARMPID_SER_B_MODEM_CONTROL         REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_MODEM_CONTROL_o)
#define ARMPID_SER_B_LINE_STATUS           REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_LINE_STATUS_o)
#define ARMPID_SER_B_MODEM_STATUS          REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_MODEM_STATUS_o)
#define ARMPID_SER_B_SCRATCHPAD            REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_SCRATCHPAD_o)
#define ARMPID_SER_B_DIVISOR_LSB           REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_DIVISOR_LSB_o)
#define ARMPID_SER_B_DIVISOR_MSB           REG_PTR(ARMPID_SER_B_BASE + ARMPID_SER_DIVISOR_MSB_o)

/*
 * Parallel Port Registers
 */
#define ARMPID_PAR_DATA                    REG_PTR(ARMPID_PAR_BASE + 0x00)
#define ARMPID_PAR_STATUS                  REG_PTR(ARMPID_PAR_BASE + 0x04)
#define ARMPID_PAR_IO_SELECT               REG_PTR(ARMPID_PAR_BASE + 0x04)
#define ARMPID_PAR_COMMAND                 REG_PTR(ARMPID_PAR_BASE + 0x08)
#define ARMPID_PAR_CONTROL                 REG_PTR(ARMPID_PAR_BASE + 0x08)

/*
 * Serial Port Interrupt Enable Register Bit fields
 */
#define ARMPID_SER_RX_HOLDING_INT_ENABLE   0x01
#define ARMPID_SER_TX_HOLDING_INT_ENABLE   0x02
#define ARMPID_SER_RX_LINE_STAT_INT_ENABLE 0x04
#define ARMPID_SER_MODEM_STAT_INT_ENABLE   0x08

/*
 * Serial Port FIFO Control Register Bit fields
 */
#define ARMPID_SER_FIFO_ENABLE             0x01
#define ARMPID_SER_RX_FIFO_RESET           0x02
#define ARMPID_SER_TX_FIFO_RESET           0x04
#define ARMPID_SER_DMA_MODE_SELECT         0x08
#define ARMPID_SER_DMA_MODE_0              0x00
#define ARMPID_SER_DMA_MODE_1              0x08
#define ARMPID_SER_RX_TRIGGER_01           0x00
#define ARMPID_SER_RX_TRIGGER_04           0x40
#define ARMPID_SER_RX_TRIGGER_08           0x80
#define ARMPID_SER_RX_TRIGGER_14           0XC0

/*
 * Interrupt Status Register Bit fields
 */
#define ARMPID_SER_INT_STATUS              0x01
#define ARMPID_SER_INT_PRIORITY_MASK       0x0E
#define ARMPID_SER_INT_SRC_LSR             0x06
#define ARMPID_SER_INT_SRC_RXRDY           0x04
#define ARMPID_SER_INT_SRC_RX_TIMEOUT      0x0C
#define ARMPID_SER_INT_SRC_TXRDY           0x02
#define ARMPID_SER_INT_SRC_MSR             0x00
#define ARMPID_SER_FIFOS_ENABLED           0xC0

/*
 *  Line Control Register Bit fields
 */
#define ARMPID_SER_DATABITS_STOPBITS_MASK  0x07
#define ARMPID_SER_DATABITS_5_STOPBITS_1   0x00
#define ARMPID_SER_DATABITS_6_STOPBITS_1   0x01
#define ARMPID_SER_DATABITS_7_STOPBITS_1   0x02
#define ARMPID_SER_DATABITS_8_STOPBITS_1   0x03
#define ARMPID_SER_DATABITS_5_STOPBITS_1_5 0x04
#define ARMPID_SER_DATABITS_6_STOPBITS_2   0x05
#define ARMPID_SER_DATABITS_7_STOPBITS_2   0x06
#define ARMPID_SER_DATABITS_8_STOPBITS_2   0x07
#define ARMPID_SER_PARITY_ENABLE           0x08
#define ARMPID_SER_PARITY_MODE_MASK        0x30
#define ARMPID_SER_PARITY_ODD              0x00
#define ARMPID_SER_PARITY_EVEN             0x10
#define ARMPID_SER_PARITY_FORCE_1          0x20
#define ARMPID_SER_PARITY_FORCE_0          0x30
#define ARMPID_SER_CLEAR_BREAK             0x00
#define ARMPID_SER_SET_BREAK               0x40
#define ARMPID_SER_DIVISOR_LATCH_ENABLE    0x80

/*
 * Modem Control Register Bit fields
 */
#define ARMPID_SER_DTR_FORCE_HIGH          0x00
#define ARMPID_SER_DTR_FORCE_LOW           0x01
#define ARMPID_SER_RTS_FORCE_HIGH          0x00
#define ARMPID_SER_RTS_FORCE_LOW           0x02
#define ARMPID_SER_INT_OUTPUT_TRISTATE     0x00
#define ARMPID_SER_INT_OUTPUT_ENABLE       0x08
#define ARMPID_SER_NORMAL_MODE             0x00
#define ARMPID_SER_LOOPBACK_MODE           0x10
#define ARMPID_SER_POWERDOWN_MODE          0x80

/*
 * Line Status Register Bit fields
 */
#define ARMPID_SER_RX_DATA_READY           0x01
#define ARMPID_SER_OVERRUN_ERROR           0x02
#define ARMPID_SER_PARITY_ERROR            0x04
#define ARMPID_SER_FRAMING_ERROR           0x08
#define ARMPID_SER_BREAK_INTERRUPT         0x10
#define ARMPID_SER_TX_HOLDING_EMPTY        0x20
#define ARMPID_SER_TX_EMPTY                0x40
#define ARMPID_FIFO_ERROR                  0x80

/*
 * Modem Status Register Bit fields
 */
#define ARMPID_SER_DELTA_CTS               0x01
#define ARMPID_SER_DELTA_DSR               0x02
#define ARMPID_SER_DELTA_RI                0x04
#define ARMPID_SER_DELTA_CD                0x08
#define ARMPID_SER_CTS                     0x10
#define ARMPID_SER_DSR                     0x20
#define ARMPID_SER_RI                      0x40
#define ARMPID_SER_CD                      0x80

/*
 * Parallel Control Register Bit fields
 */
#define ARMPID_PAR_CONTROL_STROBE          0x01
#define ARMPID_PAR_CONTROL_AUTO_FDXT       0x02
#define ARMPID_PAR_CONTROL_INIT            0x04
#define ARMPID_PAR_CONTROL_SLCTIN          0x08
#define ARMPID_PAR_CONTROL_INTP_ENABLE     0x10
#define ARMPID_PAR_CONTROL_DATA_DIR        0x20

/*
 * Parallel Command Register Bit fields
 */
#define ARMPID_PAR_COMMAND_STROBE          0x01
#define ARMPID_PAR_COMMAND_AUTO_FDXT       0x02
#define ARMPID_PAR_COMMAND_INIT            0x04
#define ARMPID_PAR_COMMAND_SLCTIN          0x08
#define ARMPID_PAR_COMMAND_INTP_ENABLE     0x10

/*
 * Parallel Status Register Bit fields
 */
#define ARMPID_PAR_STATUS_IRQ              0x04
#define ARMPID_PAR_STATUS_ERROR            0x08
#define ARMPID_PAR_STATUS_SELECT_IN        0x10
#define ARMPID_PAR_STATUS_PE               0x20
#define ARMPID_PAR_STATUS_ACK              0x40
#define ARMPID_PAR_STATUS_BUSY             0x80

#ifdef __ASSEMBLER__
/*
 * Use the following code for debugging by toggling a port pin
 */
.macro PORT_TOGGLE_DEBUG
        /*
         * Make sure we are in output mode
         */
        ldr     r0, =ARMPID_PAR_CONTROL
        ldr     r1, =0
        str     r1, [r0]

        /*
         * Leave the following values in registers:
         *
         *    r0 = Data Register
         *    r1 = LED_ON value
         *    r2 = LED_DELAY value
         */
        ldr     r0, =ARMPID_PAR_DATA
        ldr     r1, =0xF0
        ldr     r2, =0x00
        ldr     r3, =0xFFFF

        /*
         * Turn on the LED
         */
0:      str     r1, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, IMM(1)
        bne     1b

        /*
         * Turn off the LED
         */
        str     r2, [r0]

        /*
         * Delay a while
         */
        mov     r4, r3
1:      subs    r4, r4, IMM(1)
        bne     1b

        /*
         * Repeat
         */
        b       0b
.endm
#else
#define PORT_TOGGLE_DEBUG()    asm volatile ("\
        /* Make sure we are in output mode.  */\n\
        ldr     r0, =0x0D800048\n\
        ldr     r1, =0\n\
        str     r1, [r0]\n\
\n\
        /*\n\
         * Leave the following values in registers:\n\
         *\n\
         *    r0 = Data Register\n\
         *    r1 = LED_ON value\n\
         *    r2 = LED_DELAY value\n\
         */\n\
        ldr     r0, =0x0D800040\n\
        ldr     r1, =0xF0\n\
        ldr     r2, =0x00\n\
        ldr     r3, =0xFFFF\n\
\n\
        /* Turn on the LED.  */\n\
0:      str     r1, [r0]\n\
\n\
        /* Delay a while.  */\n\
        mov     r4, r3\n\
1:      subs    r4, r4, #1\n\
        bne     1b\n\
\n\
        /* Turn off the LED.  */\n\
        str     r2, [r0]\n\
\n\
        /* Delay a while.  */\n\
        mov     r4, r3\n\
1:      subs    r4, r4, #1\n\
        bne     1b\n\
\n\
        /* Repeat.  */\n\
        b       0b");
#endif /* __ASSEMBLER__ */

#define UART_BASE_0 ARMPID_SER_A_BASE
#define UART_BASE_1 ARMPID_SER_B_BASE

#ifdef __ASSEMBLER__
.macro BOARD_BOOT_LED_DEBUG num
.endm
#else
extern void flash_led(int, int);
#define BOARD_BOOT_LED_DEBUG(num)

#define BOARD_BOOT_STAGE_1() BOARD_BOOT_LED_DEBUG(5)
#define BOARD_BOOT_STAGE_2() BOARD_BOOT_LED_DEBUG(6)
#define BOARD_BOOT_STAGE_3() BOARD_BOOT_LED_DEBUG(7)
#endif /* __ASSEMBLER__ */

#endif /* __ARMPID_H__ */
