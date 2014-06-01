/*
 * irq-cpu.c -- Interrupt support for FR30.
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

static void	cpu_ictrl_init(const struct bsp_irq_controller *ic);
static int	cpu_ictrl_disable(const struct bsp_irq_controller *ic,
				  int irq_nr);
static void	cpu_ictrl_enable(const struct bsp_irq_controller *ic,
				 int irq_nr);

static const struct bsp_irq_controller core_irq_controller = {
    0, 255,
    NULL,
    cpu_ictrl_init,
    cpu_ictrl_disable,
    cpu_ictrl_enable
};


static unsigned char icr_save[48];

/*
 *  Initialize FR30 interrupt controller.
 */
static void
cpu_ictrl_init(const struct bsp_irq_controller *ic)
{
    int i;

    for (i = 0; i < 48; i++)
	icr_save[i] = 16;
}


/*
 *  Disable FR30 interrupts.
 */
static int
cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned char icr;
    int  index;

    if (irq_nr >= 16 && irq_nr <= 63) {
	index = irq_nr - 16;
	icr = fr30_io->icr[index];
	if (icr < 31) {
	    fr30_io->icr[index] = 31;
	    icr_save[index] = icr;
	    return 1;
	}
    }
    return 0;
}


/*
 *  Enable FR30 interrupts.
 */
static void
cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int  index;

    if (irq_nr >= 16 && irq_nr <= 63) {
	index = irq_nr - 16;
	fr30_io->icr[index] = icr_save[index];
    }
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

