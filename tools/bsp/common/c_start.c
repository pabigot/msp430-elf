/*
 * c_start.c -- C ROM startup code for the Cygnus BSP.
 *              This is called by the BSP boot code itself, so it sets
 *              up more than just the C runtime stuff.  It also initializes
 *              the bsp.
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
#include <bsp/cpu.h>
#include <bsp_start.h>
#ifdef __BOARD_HEADER__
#include __BOARD_HEADER__
#endif

void c_rom_start(void *config_data)
{
#ifdef BOARD_BOOT_STAGE_1
    BOARD_BOOT_STAGE_1();
#endif

    /*
     * Do the C Runtime startup stuff first part
     * This routine is shared by both the app and the bsp
     */
    c_crt0_begin();

#ifdef BOARD_BOOT_STAGE_2
    BOARD_BOOT_STAGE_2();
#endif

    /*
     * Do the bsp specific startup stuff.
     */
    c_crt0_bsp_specific(config_data);

#ifdef BOARD_BOOT_STAGE_3
    BOARD_BOOT_STAGE_3();
#endif

    /*
     * Do the C Runtime startup stuff end part
     * This routine is shared by both the app and the bsp
     */
    c_crt0_end();
}
