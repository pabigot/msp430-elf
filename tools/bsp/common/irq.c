/*
 * irq.c -- Top-level interface to interrupt control layer.
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

/*
 *  Find an interrupt controller which controls the given irq.
 */
static const struct bsp_irq_controller *
find_ictrl(int irq_nr)
{
    int i;
    const struct bsp_irq_controller *ic;

    for (i = 0; i < BSP_MAX_IRQ_CONTROLLERS; i++) {
	ic = bsp_shared_data->__ictrl_table[i];
	if (ic == NULL)
	    break;
	if (ic->first <= irq_nr && irq_nr <= ic->last)
	    return ic;
    }
    return NULL;
}


/*
 *  Enable an interrupt.
 */
void
bsp_enable_irq(int irq_nr)
{
    const struct bsp_irq_controller *ic;

    if ((ic = find_ictrl(irq_nr)) != NULL)
	ic->enable(ic, irq_nr - ic->first);
#if 0
    else
	bsp_printf("bsp_enable_irq: couldn't find controller for irq %d.\n", irq_nr);
#endif
}


/*
 *  Disable an interrupt.
 *  Returns zero if IRQ was already disabled, non-zero otherwise.
 */
int
bsp_disable_irq(int irq_nr)
{
    const struct bsp_irq_controller *ic;

    if ((ic = find_ictrl(irq_nr)) != NULL)
	return ic->disable(ic, irq_nr - ic->first);

    return 0;
}


/*
 *  Install an ISR or exception handler vector.
 *  Supported op values are:
 *      BSP_VEC_REPLACE -- completely replace existing chain.
 *      BSP_VEC_CHAIN_FIRST -- install at head of vector chain.
 *      BSP_VEC_CHAIN_LAST -- install at tail of vector chain.
 *
 *  Return value is only meaningful in the BSP_VEC_REPLACE case. In this case,
 *  the return value is a pointer to the head of the replaced chain.
 */  
bsp_vec_t *
bsp_install_vec(int vec_kind, int vec_nr, int op, bsp_vec_t *vec)
{
    const struct bsp_irq_controller *ic;
    bsp_vec_t **list, *end, *old = NULL;

    switch (vec_kind) {
      case BSP_VEC_EXCEPTION:
	list = bsp_shared_data->__exc_table;
	break;

      case BSP_VEC_INTERRUPT:
	if ((ic = find_ictrl(vec_nr)) == NULL)
	    return NULL;

	/* Get controller local irq_nr */
	vec_nr -= ic->first;
	list = ic->vec_list;
	break;

      default:
	return NULL;
    }

    switch (op) {
      case BSP_VEC_REPLACE:
	old = list[vec_nr];
	list[vec_nr] = vec;
	break;

      case BSP_VEC_CHAIN_FIRST:
	for (end = vec; end && end->next; end = end->next)
	    ;
	end->next = list[vec_nr];
	list[vec_nr] = vec;
	break;

      case BSP_VEC_CHAIN_LAST:
	for (end = list[vec_nr]; end && end->next; end = end->next)
	    ;
	if (end)
	    end->next = vec;
	else
	    list[vec_nr] = vec;
	break;
    }    
    return old;
}


/*
 *  Remove a given vector from its chain.
 */
void
bsp_remove_vec(int vec_kind, int vec_nr, bsp_vec_t *vec)
{
    const struct bsp_irq_controller *ic;
    bsp_vec_t **list, *cur, *prev;

    switch (vec_kind) {
      case BSP_VEC_EXCEPTION:
	list = bsp_shared_data->__exc_table;
	break;

      case BSP_VEC_INTERRUPT:
	if ((ic = find_ictrl(vec_nr)) == NULL)
	    return;

	/* Get controller local irq_nr */
	vec_nr -= ic->first;
	list = ic->vec_list;
	break;

      default:
	return;
    }

    for (cur = list[vec_nr], prev = NULL;
	 cur && cur != vec;
	 prev = cur, cur = cur->next)
	;

    if (cur) {
	if (prev)
	    prev->next = cur->next;
	else
	    list[vec_nr] = cur->next;
    }
}

