/*
 * irq-cpu.c -- Interrupt support for FR-V.
 *
 * Copyright 1999, 2000 Red Hat, Inc.
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
#include <bsp/cpu.h>
#include "fr500.h"
#include "bsp_if.h"

#if USE_MB86941
#include "mb86941.h"
#endif

static void	cpu_ictrl_init(const struct bsp_irq_controller *ic);
static int	cpu_ictrl_disable(const struct bsp_irq_controller *ic,
				  int irq_nr);
static void	cpu_ictrl_enable(const struct bsp_irq_controller *ic,
				 int irq_nr);

static const struct bsp_irq_controller core_irq_controller = {
    BSP_EXC_INT1, BSP_EXC_INT15,
    NULL,
    cpu_ictrl_init,
    cpu_ictrl_disable,
    cpu_ictrl_enable
};


/*
 *  Initialize FR-V interrupt controller.
 */
static void
cpu_ictrl_init(const struct bsp_irq_controller *ic)
{
#if USE_MB86941
    /* mask individual interrupts, unmask IRL */
    mb86941_write(DB_BASE, MB86941_IRQMASK, 0xfffe);
#endif
#ifdef __CPU_TIGER__
    comp_write(COMP_MASK, 0xfffe0000);
#endif
}


/*
 *  Disable FR-V interrupts.
 */
static int
cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if USE_MB86941
    unsigned int mask;

    mask = mb86941_read(DB_BASE, MB86941_IRQMASK);
    mb86941_write(DB_BASE, MB86941_IRQMASK, mask | (2 << irq_nr));

    return ((mask & (2 << irq_nr)) == 0);
#elif defined(__CPU_TIGER__)
    unsigned int mask;

    mask = comp_read(COMP_MASK);
    comp_write(COMP_MASK, mask | (0x20000 << irq_nr));

    return ((mask & (0x20000 << irq_nr)) == 0);
#else
    return 0;
#endif
}


/*
 *  Enable MIPS interrupts.
 */
static void
cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if USE_MB86941
    unsigned int mask;

    mask = mb86941_read(DB_BASE, MB86941_IRQMASK);
    mask &= ~(2 << irq_nr);
    mb86941_write(DB_BASE, MB86941_IRQMASK, mask);
#endif
#ifdef __CPU_TIGER__
    comp_write(COMP_MASK, comp_read(COMP_MASK) & ~(0x20000 << irq_nr));
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

