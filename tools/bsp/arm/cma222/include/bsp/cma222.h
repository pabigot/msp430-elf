/*
 * cma222.h -- Header for Cogent CMA222/CMA110 ARM(R) Eval Board
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
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 */
#ifndef __CMA222_H__
#define __CMA222_H__ 1

#include <bsp/bsp.h>
#include <bsp/arm710t.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     0x00000000
#define RAM_VIRTUAL_BASE    0x00000000
#define RAM_TOTAL_SIZE      (8 * 1024 * 1024)

/*
 * Flash Definitions
 */
#define FLASH_ACTUAL_BASE   0x00040000
#define FLASH_TOTAL_SIZE    SZ_256K
#define FLASH_VIRTUAL_BASE  0x00040000


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


#define CMA_IRQ_NMI      0
#define CMA_IRQ_SERIAL1  1
#define CMA_IRQ_SERIAL0  2
#define CMA_IRQ_TIMER    3
#define CMA_IRQ_PARALLEL 4
#define CMA_IRQ_SLOT1    5
#define CMA_IRQ_SLOT2    6
#define CMA_IRQ_SLOT3    7

#define CMA_IRQ_MIN 0
#define CMA_IRQ_MAX 7

#define NUM_CMA_INTERRUPTS 8

#define CMA_ICTL_SRC     ((volatile unsigned char *)0x0f600000)
#define CMA_ICTL_CLEAR   ((volatile unsigned char *)0x0f600008)
#define CMA_ICTL_MASK_RD ((volatile unsigned char *)0x0f600010)
#define CMA_ICTL_MASK_WR ((volatile unsigned char *)0x0f600018)
#define CMA_ICTL_ACK1    ((volatile unsigned char *)0x0f600020)
#define CMA_ICTL_ACK2    ((volatile unsigned char *)0x0f600028)
#define CMA_ICTL_ACK3    ((volatile unsigned char *)0x0f600030)

#endif /* __AEB_1_H__ */
