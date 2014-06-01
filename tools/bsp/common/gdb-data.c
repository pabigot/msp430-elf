/*
 * gdb-data.c -- Get pointer to shared data specific to gdb stub.
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
#include <bsp/gdb-data.h>
#include "bsp_if.h"

/*
 * Return a pointer to gdb-specific data.
 */
gdb_data_t *
__get_gdb_data(void)
{
    return (gdb_data_t *)(bsp_shared_data->__dbg_data);
}

