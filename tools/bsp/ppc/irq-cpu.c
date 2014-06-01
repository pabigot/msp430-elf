/*
 * irq-cpu.c -- Interrupt support for PowerPC.
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
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "bsp_if.h"

static void	cpu_ictrl_init(const struct bsp_irq_controller *ic);
static int	cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr);
static void	cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr);

static bsp_vec_t *cpu_vec_tbl[1];

static const struct bsp_irq_controller core_irq_controller = {
    0, 0,
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
    int		done;

    vec = core_irq_controller.vec_list[0];

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(BSP_IRQ_CORE, regs);
	vec = vec->next;
    }

    return done;
}


/*
 *  Initialize core PPC interrupt controller.
 */
static void
cpu_ictrl_init(const struct bsp_irq_controller *ic)
{
    static bsp_vec_t irq_vec;

    ic->vec_list[0] = NULL;

    irq_vec.handler = (void *)cpu_irq_dispatch;
    irq_vec.next = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_EXTIRQ,
		    BSP_VEC_REPLACE, &irq_vec);
}


/*
 *  Disable core PPC interrupts.
 */
static int
cpu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int  msr;

    asm volatile (
        "mfmsr   %0 \n\t"
	"rlwinm	 1,%0,0,17,15 \n\t"
	"mtmsr   1 \n\t"
	: "=r" (msr)
	: /* no inputs */
	: "1" );

    return ((msr & 0x8000) != 0);
}


/*
 *  Enable core PPC interrupts.
 */
static void
cpu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    asm volatile (
        "mfmsr   3 \n\t"
	"ori     3,3,0x8000 \n\t"
	"mtmsr   3 \n\t"
	: /* no outputs */
	: /* no inputs */ );
}


#if defined(__CPU_MPC8XX__)
/*
 *  The MPC8XX family also provide on chip System Interface Unit (SIU)
 *  and a Communication Processor Module (CPM). Each of these units
 *  have their own interrupt controllers.
 */
static bsp_vec_t *siu_vec_tbl[16];
static bsp_vec_t *cpm_vec_tbl[32];

static void	siu_ictrl_init(const struct bsp_irq_controller *ic);
static int	siu_ictrl_disable(const struct bsp_irq_controller *ic,
				  int irq_nr);
static void	siu_ictrl_enable(const struct bsp_irq_controller *ic,
				 int irq_nr);

static void	cpm_ictrl_init(const struct bsp_irq_controller *ic);
static int	cpm_ictrl_disable(const struct bsp_irq_controller *ic,
				  int irq_nr);
static void	cpm_ictrl_enable(const struct bsp_irq_controller *ic,
				 int irq_nr);


/* MPC8xx System Interface Unit interrupt controller. */
static const struct bsp_irq_controller siu_irq_controller = {
    1, 16,
    &siu_vec_tbl[0],
    siu_ictrl_init,
    siu_ictrl_disable,
    siu_ictrl_enable
};

/* MPC8xx Communications Processor interrupt controller. */
static const struct bsp_irq_controller cpm_irq_controller = {
    17, 48,
    &cpm_vec_tbl[0],
    cpm_ictrl_init,
    cpm_ictrl_disable,
    cpm_ictrl_enable,
};


/*
 *  ISR for MPC8xx SIU interrupts.
 */
static int
siu_irq_dispatch(int irq_nr, ex_regs_t *regs)
{
    bsp_vec_t	*vec;
    int		done, siu_irq, siu_vec;
    const struct bsp_irq_controller *ic = &siu_irq_controller;

    /* calc siu irq nr from bsp irq nr */
    siu_vec = (eppc_base()->siu_sivec >> 26) & 0x1f;
    siu_irq = 15 - siu_vec;

    /* get head of vector list */
    vec = ic->vec_list[siu_irq];

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(siu_irq + ic->first, regs);
	vec = vec->next;
    }

    eppc_base()->siu_sipend = (1 << (siu_irq + 16));

    return done;
}


/*
 *  Initialize SIU interrupt controller.
 */
static void
siu_ictrl_init(const struct bsp_irq_controller *ic)
{
    int i;
    static bsp_vec_t siu_vec;
    EPPC *eppc = eppc_base();

    for (i = 0; i <= (ic->last - ic->first); i++)
	ic->vec_list[i] = NULL;

    /* Clear pending IRQ bits. */
    eppc->siu_sipend = 0xaaaaaaaa;

    /* all siu irqs masked */
    eppc->siu_simask = 0;

    /* trigger on falling edge. */
    eppc->siu_siel = 0xaaaaaaaa;

    siu_vec.handler = (void *)siu_irq_dispatch;
    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_CORE,
		    BSP_VEC_REPLACE, &siu_vec);
}


/*
 *  Disable an SIU interrupt.
 */
static int
siu_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int ibit = (1 << (irq_nr + 16));

    if ((eppc_base()->siu_simask & ibit) == 0)
	return 0; /* already disabled */

    eppc_base()->siu_simask &= ~ibit;
    return 1;
}


/*
 *  Enable an SIU interrupt.
 */
static void
siu_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    eppc_base()->siu_simask |= (1 << (irq_nr + 16));
}


/*
 *  ISR for MPC8xx CPM interrupts.
 */
static int
cpm_irq_dispatch(int irq_nr, ex_regs_t *regs)
{
    bsp_vec_t	*vec;
    int		done, cpm_irq;
    EPPC        *eppc = eppc_base();
    const struct bsp_irq_controller *ic = &cpm_irq_controller;

    /* Set the IACK bit */
    eppc->cpmi_civr |= 1;

    /* get cpm irq nr */
    cpm_irq = (eppc->cpmi_civr >> 11) & 31;

    /* get head of vector list */
    vec = ic->vec_list[cpm_irq];

    done = 0;
    while (!done && vec && vec->handler != NULL) {
	done = vec->handler(cpm_irq + ic->first, regs);
	vec = vec->next;
    }

    return done;
}


/*
 *  Initialize CPM interrupt controller.
 */
static void
cpm_ictrl_init(const struct bsp_irq_controller *ic)
{
    int i;
    static bsp_vec_t cpm_vec;
    EPPC *eppc = eppc_base();

    for (i = 0; i <= (ic->last - ic->first); i++)
	ic->vec_list[i] = NULL;

    /*
     *  Setup CPM interrupt registers.
     *  (Section 16.20.5 of MPC860 UM)
     */
    eppc->cpmi_cicr = 0x00e41f80 | (((16 - BSP_IRQ_CPM)/2) << 13);
    eppc->cpmi_cimr = 0;
    eppc->cpmi_cipr = ~0;

    cpm_vec.handler = (void*)cpm_irq_dispatch;

    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_CPM,
		    BSP_VEC_REPLACE, &cpm_vec);
}


/*
 *  Disable a CPM interrupt.
 */
static int
cpm_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int ibit = (1 << irq_nr);
    EPPC *eppc = eppc_base();

    if ((eppc->cpmi_cimr & ibit) == 0)
	return 0;

    eppc->cpmi_cimr &= ~ibit;

    return 1;
}

/*
 *  Enable a CPM interrupt.
 */
static void
cpm_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    eppc_base()->cpmi_cimr |= (1 << irq_nr);
}
#endif /* defined(__CPU_MPC8XX__) */


#if defined(__CPU_PPC4XX__)
/*
 *  The PPC403 provides an on chip interrupt controller.
 */
static bsp_vec_t *x403_vec_tbl[31];

static void	x403_ictrl_init(const struct bsp_irq_controller *ic);
static int	x403_ictrl_disable(const struct bsp_irq_controller *ic,
				   int irq_nr);
static void	x403_ictrl_enable(const struct bsp_irq_controller *ic,
				  int irq_nr);

/* PPC4xx Interrupt controller. */
static const struct bsp_irq_controller x403_irq_controller = {
    1, 31,
    &x403_vec_tbl[0],
    x403_ictrl_init,
    x403_ictrl_disable,
    x403_ictrl_enable
};

/*
 *  ISR for PPC403 interrupts.
 */
static int
x403_irq_dispatch(int irq_nr, ex_regs_t *regs)
{
    bsp_vec_t	*vec;
    int		done = 0, irq, imask;
    unsigned    exisr;
    const struct bsp_irq_controller *ic = &x403_irq_controller;

    exisr = __get_exisr();

    for (imask = 1, irq = 31; irq && !(imask & exisr); imask <<= 1, --irq)
	;

    if (irq) {
	/* get head of vector list */
	vec = ic->vec_list[irq - 1];

	done = 0;
	while (!done && vec && vec->handler != NULL) {
	    done = vec->handler(irq, regs);
	    vec = vec->next;
	}
	__set_exisr(imask);
    }

    return done;
}


/*
 *  Initialize X403 interrupt controller.
 */
static void
x403_ictrl_init(const struct bsp_irq_controller *ic)
{
    int i;
    static bsp_vec_t x403_vec;

    for (i = 0; i <= (ic->last - ic->first); i++)
	ic->vec_list[i] = NULL;

    /* Clear pending IRQ bits. */
    __set_exisr(0xffffffff);

    /* all x403 irqs masked */
    __set_exier(0);

    x403_vec.handler = (void *)x403_irq_dispatch;
    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_IRQ_CORE,
		    BSP_VEC_REPLACE, &x403_vec);
}


/*
 *  Disable an X403 interrupt.
 */
static int
x403_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
    int      ibit = (1 << (30 - irq_nr));
    unsigned u;

    u = __get_exier();
    if ((u & ibit) == 0)
	return 0; /* already disabled */

    u &= ~ibit;
    __set_exier(u);
    return 1;
}


/*
 *  Enable an X403 interrupt.
 */
static void
x403_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
    unsigned u;

    u = __get_exier();
    u |= (1 << (30 - irq_nr));
    __set_exier(u);
}

#endif  /* __CPU_PPC4XX__ */

/*
 *  Architecture specific routine to 'install' interrupt controllers.
 */
void
_bsp_install_cpu_irq_controllers(void)
{
    /* cpu-level controller[s] */
    _bsp_install_irq_controller(&core_irq_controller);
#if defined(__CPU_MPC8XX__)
    _bsp_install_irq_controller(&siu_irq_controller);
    _bsp_install_irq_controller(&cpm_irq_controller);
#endif
#if defined(__CPU_PPC4XX__)
    _bsp_install_irq_controller(&x403_irq_controller);
#endif
}

