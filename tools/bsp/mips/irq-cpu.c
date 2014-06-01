/*
 * irq-cpu.c -- Interrupt support for MIPS.
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
#include <stdlib.h>
#include "bsp_if.h"
#include "insn.h"
#include "gdb.h"

#if defined(__CPU_R3904__)
/* R3904 implements a more complicated first-level interrupt controller */
#define CORE_IRQS 19
#else
/* 'normal' mips implementation of 6 hw, 2 sw irqs */
#define CORE_IRQS 8
#endif

static void	cpu_ictrl_init(const struct bsp_irq_controller *ic);
static int	cpu_ictrl_disable(const struct bsp_irq_controller *ic,
				  int irq_nr);
static void	cpu_ictrl_enable(const struct bsp_irq_controller *ic,
				 int irq_nr);

static bsp_vec_t *cpu_vec_tbl[CORE_IRQS];

static const struct bsp_irq_controller core_irq_controller = {
    0, CORE_IRQS - 1,
    &cpu_vec_tbl[0],
    cpu_ictrl_init,
    cpu_ictrl_disable,
    cpu_ictrl_enable
};


/*
 * Dispatch code for interrupts. Called by exception dispatch code.
 */
static int
cpu_irq_dispatch(int ex_nr, ex_regs_t *regs)
{
    bsp_vec_t	*vec;
    int		done, pending, i;

#if defined(__CPU_R3904__)
    pending = (regs->_cause >> 8) & 0xff;

    if (pending & 1)
	i = BSP_IRQ_SW0;
    else if (pending & 2)
	i = BSP_IRQ_SW1;
    else if (pending & 0x80)
	i = BSP_IRQ_HW0;
    else
	i = BSP_IRQ_HW1 + ((pending >> 2) & 15);

    vec = core_irq_controller.vec_list[i];
    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(i, regs);
	vec = vec->next;
    }

    if (i < 2)
	regs->_cause &= ~(i << 8);
    else if (i < 10)
	R3904_IACK = i - 2;
    return done;
#else
    int mask;

    mask = (regs->_sr >> 8) & 0xff;
    pending = (regs->_cause >> 8) & mask;

    for (i = 0; i < 8; i++) {
	if (pending & 1) {
	    vec = core_irq_controller.vec_list[i];

	    done = 0;
	    while (!done && vec && vec->handler != NULL) {
		done = vec->handler(i, regs);
		vec = vec->next;
	    }
	    return done;
	}
	pending >>= 1;
    }
    return 0;
#endif
}


/*
 *  Initialize MIPS interrupt controller.
 */
static void
cpu_ictrl_init(const struct bsp_irq_controller *ic)
{
    int i;
    static bsp_vec_t irq_vec;

    for (i = ic->first; i < ic->last; i++)
	ic->vec_list[i - ic->first] = NULL;

    irq_vec.handler = (void *)cpu_irq_dispatch;
    irq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_INT,
		    BSP_VEC_REPLACE, &irq_vec);
}


/*
 *  Disable MIPS interrupts.
 */
static int
cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int  sr, bit;

#if defined(__CPU_R3904__)
    int shift, prio;

    if (irq_nr == BSP_IRQ_HW0) /* can't disable */
	return 1;

    if (irq_nr >= BSP_IRQ_HW1) { /* not a sw interrupt */
	irq_nr -= BSP_IRQ_HW1;
	shift = (irq_nr & 3) * 8;
	prio = (R3904_ILRBASE[irq_nr / 4] >> shift) & 7;
	R3904_ILRBASE[irq_nr / 4] &= ~(7 << shift);
	return (prio > 0);
    }
#endif
    sr =  __get_sr();
    bit = (1 << (8 + irq_nr));
    __set_sr(sr & ~bit);

    return ((sr & bit) != 0);
}


/*
 *  Enable MIPS interrupts.
 */
static void
cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int  sr;

#if defined(__CPU_R3904__)
    if (irq_nr == BSP_IRQ_HW0) /* can't disable */
	return;

    if (irq_nr >= BSP_IRQ_HW1) { /* not a sw interrupt */
	irq_nr -= BSP_IRQ_HW1;
	R3904_ILRBASE[irq_nr / 4] |= (7 << ((irq_nr & 3) << 3));
	return;
    }
#endif

    sr = __get_sr();
    sr |= (1 << (8 + irq_nr));
    __set_sr(sr);
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


static inline void __set_cause(unsigned val)
{
    asm volatile (
        "mtc0   %0,$13\n"
	: /* no outputs */
	: "r" (val)  );
}

static inline unsigned __get_cause(void)
{
    unsigned retval;

    asm volatile (
        "mfc0   %0,$13\nnop\n"
	: "=r" (retval)
	: /* no inputs */  );

    return retval;
}

/*
 *
 */
int
_bsp_check_unexpected_irq(int irq_nr)
{
    bsp_vec_t	*vec;
    int		done;

    vec = core_irq_controller.vec_list[irq_nr];

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(irq_nr, NULL);
	vec = vec->next;
    }

    /* ack sw interrupts */
    if (irq_nr < 2)
	__set_cause(__get_cause() & ~(irq_nr << 8));

#if defined(__CPU_R3904__)
    if (irq_nr > 1 && irq_nr < 10)
	R3904_IACK = irq_nr - 2;
#endif

    if (done)
	return 0;

    return TARGET_SIGNAL_INT | 0x80000000;
}


