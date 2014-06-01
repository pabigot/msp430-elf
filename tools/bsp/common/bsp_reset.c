/*
 * bsp_reset.c -- BSP Reset Interfaces.
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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

#include "bsp_if.h"

void
bsp_reset()
{
    if (bsp_shared_data->version > 1)
	bsp_shared_data->__reset();
    else
	((bsp_v1_shared_t *)bsp_shared_data)->__reset();
}
