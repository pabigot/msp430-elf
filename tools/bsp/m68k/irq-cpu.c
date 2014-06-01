/*
 * irq-cpu.c -- Interrupt support for M68K.
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

#define DEBUG_INT_CODE 0

#ifdef __CPU_MC68328__

static void	dball_ictrl_init(const struct bsp_irq_controller *ic);
static int	dball_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr);
static void	dball_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr);

static bsp_vec_t *dball_vec_tbl[BSP_68328_IRQ_LAST + 1];

static const struct bsp_irq_controller dball_irq_controller = {
    BSP_68328_IRQ_FIRST, BSP_68328_IRQ_LAST,
    &dball_vec_tbl[0],
    dball_ictrl_init,
    dball_ictrl_disable,
    dball_ictrl_enable
};


/*
 * Dispatch code for Dragonball interrupts. Called by exception dispatch code.
 */
static int
dball_irq_dispatch(int ex_nr, ex_regs_t *regs)
{
    bsp_vec_t *vec;
    int done = 0, irq;
    register unsigned long istat = *MC68328_ISR;

#if DEBUG_INT_CODE
    bsp_printf("dball_irq_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
#endif /* DEBUG_INT_CODE */

    /*
     * Map from the 68000 Core autovector to the Dragonball System
     * Integration Module interrupt vector
     *
     * Note that if external devices require further processing
     * than the Dragonball System Integration Module registers
     * allow (ie the device has its own interrupt control registers)
     * then it should be handled by a board specific interrupt
     * controller.
     */
    switch (ex_nr)
    {
    case BSP_CORE_EXC_LEVEL_1_AUTO: irq = BSP_68328_IRQ_IRQ1; break;
    case BSP_CORE_EXC_LEVEL_2_AUTO: irq = BSP_68328_IRQ_IRQ2; break;
    case BSP_CORE_EXC_LEVEL_3_AUTO: irq = BSP_68328_IRQ_IRQ3; break;
    case BSP_CORE_EXC_LEVEL_5_AUTO: irq = BSP_68328_IRQ_PEN;  break;
    case BSP_CORE_EXC_LEVEL_7_AUTO: irq = BSP_68328_IRQ_IRQ7; break;

    case BSP_CORE_EXC_LEVEL_4_AUTO:
        /*
         * IRQ4 can be any of:
         *      General purpose interrupt INT0-INT7
         *      PWM Period Rollover
         *      Keyboard interrupt
         *      Real time clock interrupt
         *      Watchdog timer interupt
         *      UART interrupt
         *      SPI Master needs service
         *
         * Let's find out which one it is.
         *
         * Note that this automatically assigns a priority to these
         * interrupts.
         */
#ifdef INT_MASK
#error MULTIPLE DEFINITIONS OF INT_MASK
#else
#define INT_MASK(n) BSP_68328_INT_MASK((n) - BSP_68328_IRQ_FIRST)
#endif
        if      (istat & INT_MASK(BSP_68328_IRQ_INT0))  irq = BSP_68328_IRQ_INT0;
        else if (istat & INT_MASK(BSP_68328_IRQ_INT1))  irq = BSP_68328_IRQ_INT1;
        else if (istat & INT_MASK(BSP_68328_IRQ_INT2))  irq = BSP_68328_IRQ_INT2;
        else if (istat & INT_MASK(BSP_68328_IRQ_INT3))  irq = BSP_68328_IRQ_INT3;
        else if (istat & INT_MASK(BSP_68328_IRQ_INT4))  irq = BSP_68328_IRQ_INT4;
        else if (istat & INT_MASK(BSP_68328_IRQ_INT5))  irq = BSP_68328_IRQ_INT5;
        else if (istat & INT_MASK(BSP_68328_IRQ_INT6))  irq = BSP_68328_IRQ_INT6;
        else if (istat & INT_MASK(BSP_68328_IRQ_INT7))  irq = BSP_68328_IRQ_INT7;
        else if (istat & INT_MASK(BSP_68328_IRQ_PWM))   irq = BSP_68328_IRQ_PWM;
        else if (istat & INT_MASK(BSP_68328_IRQ_KB))    irq = BSP_68328_IRQ_KB;
        else if (istat & INT_MASK(BSP_68328_IRQ_RTC))   irq = BSP_68328_IRQ_RTC;
        else if (istat & INT_MASK(BSP_68328_IRQ_WDT))   irq = BSP_68328_IRQ_WDT;
        else if (istat & INT_MASK(BSP_68328_IRQ_UART))  irq = BSP_68328_IRQ_UART;
        else if (istat & INT_MASK(BSP_68328_IRQ_TMR2))  irq = BSP_68328_IRQ_TMR2;
        else if (istat & INT_MASK(BSP_68328_IRQ_SPIM))  irq = BSP_68328_IRQ_SPIM;
        else
            /*
             * Unknown level 4 interrupt
             */
            return 0;

        break;

    case BSP_CORE_EXC_LEVEL_6_AUTO:
        /*
         * IRQ6 can be any of:
         *      External IRQ6 interrupt
         *      Timer 1 event
         *      SPI slave needs service
         *
         * Let's find out which one it is.
         *
         * Note that this automatically assigns a priority to these
         * interrupts.
         */
        if      (istat & INT_MASK(BSP_68328_IRQ_TMR1)) irq = BSP_68328_IRQ_TMR1;
        else if (istat & INT_MASK(BSP_68328_IRQ_SPIS)) irq = BSP_68328_IRQ_SPIS;
        else if (istat & INT_MASK(BSP_68328_IRQ_IRQ6)) irq = BSP_68328_IRQ_IRQ6;
        else
            /*
             * Unknown level 6 interrupt
             */
            return 0;

        break;
#undef INT_MASK

    default:
        /*
         * Unknown interrupt.
         */
        return 0;
    }

    vec = dball_vec_tbl[irq - BSP_68328_IRQ_FIRST];
#if DEBUG_INT_CODE
    bsp_printf("irq = %d\n", irq);
#endif /* DEBUG_INT_CODE */

    while (!done && vec && vec->handler != NULL) {
#if DEBUG_INT_CODE
        bsp_printf("vec->handler = 0x%08lx\n", vec->handler);
        bsp_printf("vec->next = 0x%08lx\n", vec->next);
#endif /* DEBUG_INT_CODE */
        done = vec->handler(irq, regs);
        vec = vec->next;
    }

    return done;
}


/*
 *  Initialize Dragonball interrupt controller.
 */
static void
dball_ictrl_init(const struct bsp_irq_controller *ic)
{
    static bsp_vec_t irq_vec[BSP_CORE_EXC_LEVEL_7_AUTO - BSP_CORE_EXC_LEVEL_1_AUTO + 1];
    int i;

#if DEBUG_INT_CODE
    bsp_printf("dball_irq_init(0x%08lx)\n", ic);
#endif /* DEBUG_INT_CODE */

    /*
     * Setup the IVR so external interrupts work
     *
     * Autovector 1 = 0x19.
     * Vector Generated = level | (IVR & 0XF8)
     * Therefore IVR = 0x18
     */
    *MC68328_IVR = 0x18;
    for (i = ic->first; i < ic->last; i++)
    {
        ic->vec_list[i - ic->first] = NULL;
    }

    for (i = 0; i < 7; i++)
    {
        irq_vec[i].handler = (void *)dball_irq_dispatch;
        irq_vec[i].next = NULL;
        bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_LEVEL_1_AUTO + i,
                        BSP_VEC_CHAIN_FIRST, &irq_vec[i]);
    }
}


#ifdef INT_MASK
#error MULTIPLE DEFINITIONS OF INT_MASK
#else
#define INT_MASK(n) BSP_68328_INT_MASK(n)
#endif

/*
 *  Disable Dragonball interrupts.
 */
static int
dball_ictrl_disable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if DEBUG_INT_CODE
    bsp_printf("dball_irq_disable(0x%08lx, %d)\n", ic, irq_nr);
    bsp_printf("Oring w/ IMR: 0x%08lx\n", INT_MASK(irq_nr));
#endif /* DEBUG_INT_CODE */
    *MC68328_IMR |= INT_MASK(irq_nr);
    return 0;
}


/*
 *  Enable Dragonball interrupts.
 */
static void
dball_ictrl_enable(const struct bsp_irq_controller *ic, int irq_nr)
{
#if DEBUG_INT_CODE
    bsp_printf("dball_irq_enable(0x%08lx, %d)\n", ic, irq_nr);
    bsp_printf("Anding w/ IMR: 0x%08lx\n", ~INT_MASK(irq_nr));
#endif /* DEBUG_INT_CODE */
    *MC68328_IMR &= ~INT_MASK(irq_nr);
}

#undef INT_MASK

#endif /* __CPU_MC68328__ */

/*
 *  Architecture specific routine to 'install' interrupt controllers.
 */
void
_bsp_install_cpu_irq_controllers(void)
{
    /*
     * We are not going to install a controller for 68000 core interrupts.
     * We will just let them be handled by the exception processing
     * code in bsp.c
     */

#ifdef __CPU_MC68328__
    _bsp_install_irq_controller(&dball_irq_controller);
#endif /* __CPU_MC68328__ */
}

#if DEBUG_INT_CODE
/*
 * Print out a stack dump
 */
void stack_dump(unsigned short *sp)
{
    int i, j;

    for (j = 0; j < 8; j++)
    {
        bsp_printf("\t0x%08lx: ", sp);
        for (i = 0; i < 8; i++)
        {
            unsigned short sp_value;

            if (bsp_memory_read((void *)sp, 0, sizeof(*sp) * 8, 1, &sp_value) != 0)
                bsp_printf("%04x ", sp_value);
            else
                bsp_printf(".... ");

            sp++;
        }
        bsp_printf("\n");
    }
}

/*
 * Print some diagnostic info about the exception
 */
static void exceptionPrint(ex_regs_t *regs, unsigned long vect_num, unsigned short *stkfrm)
{
    bsp_printf("Exception #0x%x\n", vect_num);
    bsp_printf("Stack Frame:\n");
    stack_dump(stkfrm);
    bsp_printf("Register Dump (0x%08lx):\n", (unsigned long)regs);
    _bsp_dump_regs(regs);
    if ((vect_num != BSP_CORE_EXC_BUS_ERROR) &&
        (vect_num != BSP_CORE_EXC_ADDR_ERROR))
    {
        /*
         * Only do a stack dump if we are not processing
         * an abort exception already.  If we do a stack
         * dump in an exception, that will corrupt the 
         * jmpbuf already in use in generic_mem.c
         *
         * Besides, we really don't want to produce a
         * double-abort situation
         */
        bsp_printf("Stack Frame:\n");
        stack_dump((unsigned short *)(regs->_sp));
    }
}
#endif /* DEBUG_INT_CODE */

/*
 * This routine sets up the registers (SP,PC,SR) and calls _bsp_exc_dispatch
 */
m68k_grp_1_2_stkfrm *processException(unsigned short *stkfrm, 
                                      void *registers, 
                                      unsigned long vect_num)
{
    m68k_grp_1_2_stkfrm *frame;
    int frame_size;
    ex_regs_t *regs = (ex_regs_t *)registers;

#if DEBUG_INT_CODE
    {
        unsigned short *sp = (unsigned short *)&stkfrm;
        unsigned short *old_sp = (unsigned short *)((unsigned long)stkfrm - 8);

        bsp_printf("stkfrm = 0x%08lx\n", stkfrm);
        bsp_printf("register = 0x%08lx\n", registers);
        bsp_printf("vect_num = 0x%08lx\n", vect_num);
        bsp_printf("current sp = 0x%08lx\n", sp);
        bsp_printf("stack dump:\n");
        stack_dump(sp);
        bsp_printf("old stack dump:\n");
        stack_dump(old_sp);
    }
#endif /* DEBUG_INT_CODE */

    /*
     * Fill in the faulting PC and SR
     */
    if (vect_num <= BSP_CORE_EXC_ADDR_ERROR)
    {
        m68k_bus_addr_stkfrm *tmp_frame = (m68k_bus_addr_stkfrm *)stkfrm;
        frame = (m68k_grp_1_2_stkfrm *)&tmp_frame->sr;
        frame_size = sizeof(m68k_bus_addr_stkfrm);
    } else {
        frame = (m68k_grp_1_2_stkfrm *)stkfrm;
        frame_size = sizeof(m68k_grp_1_2_stkfrm);
    }
    regs->_sr = frame->sr;
    regs->_pc = frame->pc_l | (((unsigned long)frame->pc_h)<<16);

    /*
     * Adjust the PC if this is a breakpoint that GDB
     * inserted.
     */
    if (vect_num == GDB_BREAKPOINT_VECTOR)
    {
        regs->_pc -= GDB_BREAKPOINT_INST_SIZE;
        frame->pc_l = (unsigned short)(regs->_pc & 0xFFFF);
        frame->pc_h = (unsigned short)(regs->_pc >> 16);
    }

    /*
     * Fill in the faulting SP
     */
    if ((regs->_sr & 0x2000) == 0)
    {
        /*
         * We were in user mode.
         * Save the USP
         */
        regs->_sp = (unsigned long)__get_usp();    
    } else {
        /*
         * We were in supervisor mode.
         * Save the SSP upon expection entry
         */
        regs->_sp = (unsigned long)stkfrm + frame_size;
    }

#if DEBUG_INT_CODE
    exceptionPrint(regs, vect_num, stkfrm);
#endif

    /*
     * This may or may not return.
     *
     * If it does not return, then it is up to the routine called to ensure the
     * stack is back to normal and an rte is called.
     */
    if (_bsp_exc_dispatch(vect_num, registers) == 0)
    {
        unsigned orig_sr = __get_sr();

        /*
         * nobody handled this exception.
         * Let's let GDB take care of it
         */
        __set_sr(orig_sr | MC68000_SR_INT_MASK);       /* Disable all interrupts  */
        bsp_invoke_dbg_handler(vect_num, registers);
        __set_sr(orig_sr);                             /* Reenable all interrupts */
    }

    /*
     * Now, update the stack frame with the reg_image[] array
     */
    frame->sr = (unsigned short)regs->_sr;
    frame->pc_l = (unsigned short)(regs->_pc & 0xFFFF);
    frame->pc_h = (unsigned short)(regs->_pc >> 16);

#if DEBUG_INT_CODE
    bsp_printf("Returning from exception\n");
    _bsp_dump_regs(registers);
    bsp_printf("frame->sr = 0x%04lx\tframe->pc = 0x%04lx\n", 
               frame->sr, ((frame->pc_h << 16) | frame->pc_l));
#endif

    return frame;
}
