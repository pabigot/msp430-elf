/*
 * irq-cpu.c -- Interrupt support for Matsusita AM30.
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
#include "gdb.h"

static void	cpu_ictrl_init(const struct bsp_irq_controller *ic);
static int	cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr);
static void	cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr);

/*
 *  Each possible interrupt gets its own bsp_isr_desc. This
 *  simplifies the code, but is wasteful of space because
 *  most interrupt groups support only 1, not 4, interrupt
 *  sources.
 */
#define NUM_IRQS (31*4)

static bsp_vec_t *_cpu_vec_tbl[NUM_IRQS];

static const struct bsp_irq_controller core_irq_controller = {
	0, NUM_IRQS-1,
	&_cpu_vec_tbl[0],
	cpu_ictrl_init,
	cpu_ictrl_disable,
	cpu_ictrl_enable
};


/*
 *  Table to find least significant bit set in binary
 *  representation of small integers (0-15).
 */
const static char lsbit[16] = {
    -1, 0, 1, 0, 2, 0, 1, 0,
     3, 0, 1, 0, 2, 0, 1, 0 
};


/*
 *  NMI's are a little tricky in that they look a lot like normal
 *  interrupts except there is a separate HW vector.
 */
int
_handle_nmi_interrupt(ex_regs_t *regs)
{
    int            done;
    bsp_vec_t	   *vec;

    vec = _cpu_vec_tbl[BSP_IRQ_NMI];

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(BSP_IRQ_NMI, regs);
	vec = vec->next;
    }

    return done;
}


/*
 *  Watchdog interrupts come through the NMI HW vector, so
 *  we handle them similarly to NMI.
 */
int
_handle_watchdog_interrupt(ex_regs_t *regs)
{
    int            done;
    bsp_vec_t	   *vec;

    vec = _cpu_vec_tbl[BSP_IRQ_WATCHDOG];

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(BSP_IRQ_WATCHDOG, regs);
	vec = vec->next;
    }

    return done;
}


/*
 *  Initialize core AM30 interrupt controller.
 */
static void
cpu_ictrl_init(const struct bsp_irq_controller *ic)
{
    extern void _interrupt_handler(void);
    unsigned short vector;
    int i;

    /*
     * Install ROM interrupt vectors.
     */
    vector = (unsigned short)((unsigned)_interrupt_handler & 0xffff);

    for (i = 0; i < 7; i++)
	am30_ivar[i].ivar = vector;
}


/*
 *  Disable core AM30 interrupts.
 */
static int
cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int grp, bit, flags, was_set;
    unsigned short icr;

    grp = irq_nr >> 2;
    bit = irq_nr & 3;

    DISABLE_INTERRUPTS(flags);

    icr = am30_icr[grp].icr;
    if ((was_set = (icr & (0x100 << bit))) != 0)
	am30_icr[grp].icr = icr ^ (0x100 << bit);

    RESTORE_INTERRUPTS(flags);

    return was_set;
}


/*
 *  Enable core AM30 interrupts.
 */
static void
cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int grp, bit, flags;
    unsigned short icr;

    grp = irq_nr >> 2;
    bit = irq_nr & 3;

    DISABLE_INTERRUPTS(flags);

    icr = am30_icr[grp].icr;
    am30_icr[grp].icr = (icr & 0xff00) | (0x100 << bit);

#if 0
    bsp_printf("cpu_ictrl_enable: irq[%d] icr[0x%04x]\n",
	    irq_nr, am30_icr[grp].icr);
#endif

    RESTORE_INTERRUPTS(flags);
}


/*
 * Second-level dispatch code for interrupts handled through
 * the core priority vectors.
 */
int
_cpu_irq_dispatch(ex_regs_t *regs)
{
    unsigned short grp;
    unsigned short icr;
    int            irq, bit, done;
    bsp_vec_t	   *vec;

    grp = am30_iagr / 4;
    icr = am30_icr[grp].icr;
    bit = lsbit[icr & 15];
    irq = (grp * 4) + bit;

    vec = _cpu_vec_tbl[irq];

#if 0
    bsp_printf("_cpu_irq_dispatch: grp<%d> icr<%x> bit<%d> vec<%x>\n",
	    grp, icr, bit, vec);
#endif

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(irq, regs);
	vec = vec->next;
    }

    /* clear the ID and IR bits. */
    am30_icr[grp].icr |= (1 << bit);

    return done;
}


/*
 * Second-level dispatch code for NMI.
 */
int
_cpu_nmi_dispatch(ex_regs_t *regs)
{
    unsigned char *pc = (unsigned char *)regs->_pc;

    if (pc[0] == BREAKPOINT_OPCODE) {
	regs->_dummy = TARGET_SIGNAL_TRAP;
	return _bsp_exc_dispatch(BSP_EXC_BREAKPOINT, regs);
    }

    if (pc[0] == 0xf0 && (pc[1] == 0x20 || pc[1] == 0xC0)) {
	regs->_dummy = TARGET_SIGNAL_TRAP;
	return _bsp_exc_dispatch(BSP_EXC_SYSCALL, regs);
    }
    
    if (am30_isr & 4) {
	regs->_dummy = TARGET_SIGNAL_BUS;
	return _bsp_exc_dispatch(BSP_EXC_BUSERR, regs);
    }

    if (am30_isr & 0x20) {
	regs->_dummy = TARGET_SIGNAL_SEGV;
	return _bsp_exc_dispatch(BSP_EXC_MEMERR, regs);
    }

    regs->_dummy = TARGET_SIGNAL_ILL;
    return _bsp_exc_dispatch(BSP_EXC_ILLEGAL, regs);

    return 0;
}


void
_bsp_install_cpu_irq_controllers(void)
{
    /* cpu-level controller[s] */
    _bsp_install_irq_controller(&core_irq_controller);
}
