/*
 * c_crt0.c -- C Run-time Startup code for the Cygnus BSP.
 *             This is called by low-level startup code.
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

#include <bsp_start.h>

void c_start(void)
{
    /*
     * Do the C Runtime startup stuff first part
     * This routine is shared by both the app and the bsp
     */
    c_crt0_begin();

    /*
     * Do the app specific startup stuff.
     */
    c_crt0_app_specific();

    /*
     * Do the C Runtime startup stuff end part
     * This routine is shared by both the app and the bsp
     */
    c_crt0_end();
}
