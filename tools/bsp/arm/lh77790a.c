/*
 * lh77790a.h -- CPU specific code for Cygnus BSP.
 *   Sharp LH77790A
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
#include <bsp/lh77790a.h>
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "bsp_if.h"

#define DEBUG_IRQ_CODE 0

#define NUM_LH790_INTERRUPTS ((LH77790A_IRQ_MAX - LH77790A_IRQ_MIN) + 1)

static void	lh790_ictrl_init(const struct bsp_irq_controller *ic);
static int	lh790_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr);
static void	lh790_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr);

static bsp_vec_t *lh790_irq_vec_tbl[NUM_LH790_INTERRUPTS];

static const struct bsp_irq_controller lh790_irq_controller = {
    LH77790A_IRQ_MIN, LH77790A_IRQ_MAX,
    &lh790_irq_vec_tbl[0],
    lh790_ictrl_init,
    lh790_ictrl_disable,
    lh790_ictrl_enable
};

/*
 * Dispatch code for lh790 interrupts. Called by exception dispatch code.
 */
static int
lh790_irq_dispatch(int ex_nr, ex_regs_t *regs)
{
    bsp_vec_t *vec = NULL;
    int done = 0;
    int irq, status;

#if DEBUG_IRQ_CODE
    bsp_printf("lh790_irq_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
    BSP_ASSERT((ex_nr == BSP_CORE_EXC_IRQ) || (ex_nr == BSP_CORE_EXC_FIQ));

    bsp_printf("*LH790_IRQSR = 0x%04lx\n", *LH77790A_IRQSR);
    bsp_printf("*LH790_FIQSR = 0x%04lx\n", *LH77790A_FIQSR);
    bsp_printf("*LH790_IPR   = 0x%04lx\n", *LH77790A_IPR);
#endif /* DEBUG_IRQ_CODE */

    /*
     * Some FIQ or IRQ occurred.
     * Figure out which one.
     */
    if (ex_nr == BSP_CORE_EXC_FIQ)
        status = *LH77790A_FIQSR;
    else
        status = *LH77790A_IRQSR;

    /*
     * We will prioritize them by the low-order bit that is
     * set in the status register
     */
    for (irq = LH77790A_IRQ_MIN; irq <= LH77790A_IRQ_MAX; irq++)
    {
	if (status & 1)
	    break;
	status >>= 1;
    }

#if DEBUG_IRQ_CODE
    bsp_printf("irq == %d\n", irq);
#endif /* DEBUG_IRQ_CODE */

    if (irq <= LH77790A_IRQ_MAX)
        vec = lh790_irq_vec_tbl[irq];

    while (!done && vec && vec->handler != NULL) {
#if DEBUG_IRQ_CODE
        bsp_printf("vec->handler = 0x%08lx\n", vec->handler);
        bsp_printf("vec->next = 0x%08lx\n", vec->next);
#endif /* DEBUG_IRQ_CODE */
        done = vec->handler(irq, regs);
        vec = vec->next;
    }

    /* clear the edge triggered irqs */
    if (irq <= 8)
	*LH77790A_ICLR = (1 << irq);
    else if (irq == 12)
	*LH77790A_ICLR = (1 << 9);

    return done;
}


/*
 *  Initialize LH790 interrupt controller.
 */
static void
lh790_ictrl_init(const struct bsp_irq_controller *ic)
{
    static bsp_vec_t irq_vec, fiq_vec;
    int i;

#if DEBUG_IRQ_CODE
    bsp_printf("lh790_ictrl_init(0x%08lx)\n", ic);

    bsp_printf("*LH790_IRQSR = 0x%04lx\n", *LH77790A_IRQSR);
    bsp_printf("*LH790_FIQSR = 0x%04lx\n", *LH77790A_FIQSR);
    bsp_printf("*LH790_IRQER = 0x%04lx\n", *LH77790A_IRQER);
    bsp_printf("*LH790_FIQER = 0x%04lx\n", *LH77790A_FIQER);
    bsp_printf("*LH790_IPR   = 0x%04lx\n", *LH77790A_IPR);
#endif /* DEBUG_IRQ_CODE */

    /*
     * Clear all pending interrupts
     */
    *LH77790A_IRQER = 0;
    *LH77790A_FIQER = 0;
    *LH77790A_ICLR = 0x3ff;

    for (i = ic->first; i < ic->last; i++)
    {
        ic->vec_list[i - ic->first] = NULL;
    }

    /*
     * Setup exception handlers for the exceptions
     * corresponding to IRQ's and FIQ's
     */
    irq_vec.handler = (void *)lh790_irq_dispatch;
    irq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_IRQ, BSP_VEC_CHAIN_FIRST, &irq_vec);

    fiq_vec.handler = (void *)lh790_irq_dispatch;
    fiq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_FIQ, BSP_VEC_CHAIN_FIRST, &fiq_vec);
}


/*
 *  Disable LH790 interrupts.
 */
static int
lh790_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int old_er, mask = 1 << irq_nr;

#if DEBUG_IRQ_CODE
    BSP_ASSERT((irq_nr >= 0) && (irq_nr < NUM_LH790_INTERRUPTS));
    bsp_printf("lh790_ictrl_disable(0x%08lx, %d)\n", ic, irq_nr);

    bsp_printf("*LH790_IRQSR = 0x%04lx\n", *LH77790A_IRQSR);
    bsp_printf("*LH790_FIQSR = 0x%04lx\n", *LH77790A_FIQSR);
    bsp_printf("*LH790_IRQER = 0x%04lx\n", *LH77790A_IRQER);
    bsp_printf("*LH790_FIQER = 0x%04lx\n", *LH77790A_FIQER);
    bsp_printf("*LH790_IPR   = 0x%04lx\n", *LH77790A_IPR);
#endif /* DEBUG_IRQ_CODE */

    /* can't disable watchdog here */
    if (irq_nr == LH77790A_IRQ_WATCHDOG)
	return 0;

    if (mask & LH77790A_IRQ_MASK)
    {
	old_er = *LH77790A_IRQER;
	*LH77790A_IRQER = old_er & ~mask;
    }
    else
    {
	old_er = *LH77790A_FIQER;
	*LH77790A_FIQER = old_er & ~mask;
    }

    return ((old_er & mask) != 0);
}


/*
 *  Enable LH790 interrupts.
 */
static void
lh790_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int mask = 1 << irq_nr;

#if DEBUG_IRQ_CODE
    bsp_printf("lh790_ictrl_enable(0x%08lx, %d)\n", ic, irq_nr);
    BSP_ASSERT((irq_nr >= 0) && (irq_nr < NUM_LH790_INTERRUPTS));

    bsp_printf("*LH790_IRQSR = 0x%04lx\n", *LH77790A_IRQSR);
    bsp_printf("*LH790_FIQSR = 0x%04lx\n", *LH77790A_FIQSR);
    bsp_printf("*LH790_IRQER = 0x%04lx\n", *LH77790A_IRQER);
    bsp_printf("*LH790_FIQER = 0x%04lx\n", *LH77790A_FIQER);
    bsp_printf("*LH790_IPR   = 0x%04lx\n", *LH77790A_IPR);
#endif /* DEBUG_IRQ_CODE */

    /* can't disable watchdog here */
    if (irq_nr == LH77790A_IRQ_WATCHDOG)
	return;

    if (mask & LH77790A_IRQ_MASK)
	*LH77790A_IRQER |= mask;
    else
	*LH77790A_FIQER |= mask;
}

/*
 * LH790 specific routine to 'install' interrupt controllers.
 */
void
_bsp_install_lh790_irq_controllers(void)
{
#if DEBUG_IRQ_CODE
    bsp_printf("_bsp_install_lh790_irq_controllers()\n");
#endif /* DEBUG_IRQ_CODE */

    _bsp_install_irq_controller(&lh790_irq_controller);
}

