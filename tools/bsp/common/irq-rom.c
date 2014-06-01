/*
 * irq-rom.c -- BSP interrupt handling needed only for ROM builds.
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

#include <stdlib.h>
#include <bsp/bsp.h>
#include "bsp_if.h"

#ifndef DEBUG_DISPATCH
#define DEBUG_DISPATCH 0
#endif

/*
 *  Dispatch function for CPU exceptions.
 *  Returns true if exception was handled by installed handler,
 *  zero if not.
 */
int
_bsp_exc_dispatch(int exc_nr, void *regs)
{
    bsp_vec_t *vec;
    int done;

#if DEBUG_DISPATCH
    bsp_printf("_bsp_exc_dispatch(%d) sp(0x%x)\n", exc_nr, &vec);
#endif

    vec = bsp_shared_data->__exc_table[exc_nr];
    done = 0;
    while (!done && vec != NULL && vec->handler != NULL) {
	done = vec->handler(exc_nr, regs);
	vec = vec->next;
    }

#if DEBUG_DISPATCH
    if (!done)
	bsp_printf("Unexpected Exception [%d]\n", exc_nr);
    else
	bsp_printf("_bsp_exc_dispatch(%d) done.\n", exc_nr);
#endif

    return done;
}


/*
 * Boot-time initialization of bsp interrupt handling module.
 * Should be called from _bsp_late_init after CPU and board
 * specific initialization code has installed interrupt
 * controller descriptors.
 */
void
__init_irq_controllers(void)
{
    int i;
    const struct bsp_irq_controller *ic;

    for (i = 0; i < BSP_MAX_IRQ_CONTROLLERS; i++) {
	ic = bsp_shared_data->__ictrl_table[i];
	if (ic == NULL)
	    break;
	ic->init(ic);
    }
}


/*
 *  Install an interrupt controller descriptor.
 */
void
_bsp_install_irq_controller(const struct bsp_irq_controller *ic)
{
    int i;

    for (i = 0; i < BSP_MAX_IRQ_CONTROLLERS; i++) {
	if (bsp_shared_data->__ictrl_table[i] == NULL)
	    break;
    }
    if (i < BSP_MAX_IRQ_CONTROLLERS)
	bsp_shared_data->__ictrl_table[i] = ic;
}

