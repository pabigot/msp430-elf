/*
 * bsp_if.c -- Miscellaneous BSP Interfaces.
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

#include <bsp/bsp.h>
#include "bsp_if.h"

/*
 * Install a debug handler.
 * Returns old handler being replaced.
 */
bsp_handler_t
bsp_install_dbg_handler(bsp_handler_t new_handler)
{
    bsp_handler_t old_handler;

    old_handler = *bsp_shared_data->__dbg_vector;
    *bsp_shared_data->__dbg_vector = new_handler;

    return old_handler;
}

/*
 * Sometimes it is desireable to call the debug handler directly. This routine
 * accomplishes that. It is the responsibility of the caller to insure that
 * interrupts are disabled before calling this routine.
 */
void
bsp_invoke_dbg_handler(int exc_nr, void *regs)
{
    (*bsp_shared_data->__dbg_vector)(exc_nr, regs);
}

/*
 * Install a 'kill' handler.
 * Returns old handler being replaced.
 */
bsp_handler_t
bsp_install_kill_handler(bsp_handler_t new_handler)
{
    bsp_handler_t old_handler;

    old_handler = bsp_shared_data->__kill_vector;
    bsp_shared_data->__kill_vector = new_handler;

    return old_handler;
}


void *
bsp_cpu_data(void)
{
    if (bsp_shared_data->version > 1)
	return bsp_shared_data->__cpu_data;
    return ((bsp_v1_shared_t *)bsp_shared_data)->__cpu_data;
}


void *
bsp_board_data(void)
{
    if (bsp_shared_data->version > 1)
	return bsp_shared_data->__board_data;
    return ((bsp_v1_shared_t *)bsp_shared_data)->__board_data;
}


int
bsp_sysinfo(enum bsp_info_id id, ...)
{
    int     retval;
    va_list ap;

    va_start (ap, id);
    if (bsp_shared_data->version > 1)
	retval = bsp_shared_data->__sysinfo(id, ap);
    else
	retval = ((bsp_v1_shared_t *)bsp_shared_data)->__sysinfo(id, ap);
    va_end(ap);
    return retval;
}

int
bsp_set_debug_comm(int id)
{
    if (bsp_shared_data->version > 1)
	return bsp_shared_data->__set_debug_comm(id);
    else
	return ((bsp_v1_shared_t *)bsp_shared_data)->__set_debug_comm(id);
}

int
bsp_set_console_comm(int id)
{
    if (bsp_shared_data->version > 1)
	return bsp_shared_data->__set_console_comm(id);
    else
	return ((bsp_v1_shared_t *)bsp_shared_data)->__set_console_comm(id);
}

int
bsp_set_serial_baud(int id, int baud)
{
    if (bsp_shared_data->version > 1)
	return bsp_shared_data->__set_serial_baud(id, baud);
    else
	return ((bsp_v1_shared_t *)bsp_shared_data)->__set_serial_baud(id, baud);
}


#if !defined(NDEBUG)

void _bsp_assert(const char *file, const int line, const char *condition)
{
    bsp_printf("Assertion \"%s\" failed\n", condition);
    bsp_printf("File \"%s\"\n", file);
    bsp_printf("Line %d\n", line);
#if defined(PORT_TOGGLE_DEBUG)
    PORT_TOGGLE_DEBUG();
#else
    while(1) ;
#endif /* defined(PORT_TOGGLE_DEBUG) */
}

#endif /* !defined(NDEBUG) */

