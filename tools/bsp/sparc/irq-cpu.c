/*
 * irq-cpu.c -- Interrupt support for Sparc.
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
#include <bsp/bsp.h>
#include "bsp_if.h"
#include <stdlib.h>

#ifdef __BOARD_MB8683X__
#include "mb86940.h"
#define USE_MB86940 1
#endif

static void	cpu_ictrl_init(const struct bsp_irq_controller *ic);
static int	cpu_ictrl_disable(const struct bsp_irq_controller *ic,
				  int irq_nr);
static void	cpu_ictrl_enable(const struct bsp_irq_controller *ic,
				 int irq_nr);

static const struct bsp_irq_controller core_irq_controller = {
#if USE_MB86940
    BSP_EXC_INT1, BSP_EXC_INT15,
#else
    0, 0,
#endif
    NULL,
    cpu_ictrl_init,
    cpu_ictrl_disable,
    cpu_ictrl_enable
};


/*
 *  Initialize interrupt controller.
 */
static void
cpu_ictrl_init(const struct bsp_irq_controller *ic)
{
#if USE_MB86940
    /* mask individual interrupts, unmask IRL */
    mb86940_write(MB86940_IRQMASK, 0xfffe);
#endif
}


/*
 *  Disable interrupts.
 */
static int
cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if USE_MB86940
    unsigned int mask;

    mask = mb86940_read(MB86940_IRQMASK);
    mb86940_write(MB86940_IRQMASK, mask | (2 << irq_nr));
    return ((mask & (2 << irq_nr)) == 0);
#endif
}


/*
 *  Enable interrupts.
 */
static void
cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if USE_MB86940
    unsigned int mask;

    mask = mb86940_read(MB86940_IRQMASK);
    mask &= ~(2 << irq_nr);
    mb86940_write(MB86940_IRQMASK, mask);
#endif
}


/*
 *  Architecture specific routine to 'install' interrupt controllers.
 */
void
_bsp_install_cpu_irq_controllers(void)
{
    /* cpu-level controller[s] */
    _bsp_install_irq_controller(&core_irq_controller);
}

