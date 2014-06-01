/*
 * xaret.h -- Header for Intel(R) SA-220 Eval Board
 *
 * Copyright (c) 2000 Red Hat, Inc.
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
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 */
#ifndef __XARET_H__
#define __XARET_H__ 1

#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/defs.h>

/*
 * RAM Definitions
 */
#define RAM_ACTUAL_BASE     0xA0000000
#define RAM_VIRTUAL_BASE    0x00000000
/* #define RAM_TOTAL_SIZE      (32 * 1024*1024) */
#define RAM_TOTAL_SIZE      (1 * 1024*1024)

/*
 * Flash Definitions
 */
#define FLASH_ACTUAL_BASE   0x00000000
#define FLASH_VIRTUAL_BASE  0xA0000000
#define FLASH_TOTAL_SIZE    SZ_256K

/*
 * FLASH1 base is used for SRAM and some memory mapped IO.
 */
#define FLASH1_ACTUAL_BASE  0xB0000000
#define FLASH1_VIRTUAL_BASE 0xB0000000
#define FLASH1_TOTAL_SIZE   SZ_512K

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


#endif /* __XARET_H__ */
