/*
 * irq-cpu.c -- Interrupt support for ARM(R).
 *
 * Copyright (c) 1998, 1999, 2000 Cygnus Support
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
 *
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "bsp_if.h"
#include "insn.h"

#define DEBUG_INT_CODE 0

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
static void exceptionPrint(ex_regs_t *regs, unsigned long vect_num)
{
    bsp_printf("Exception #0x%x\n", vect_num);
    bsp_printf("Register Dump (0x%08lx):\n", (unsigned long)regs);
    _bsp_dump_regs(regs);
    if ((vect_num != BSP_CORE_EXC_PREFETCH_ABORT) &&
        (vect_num != BSP_CORE_EXC_DATA_ABORT))
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
 *  Architecture specific routine to 'install' interrupt controllers.
 */
void
_bsp_install_cpu_irq_controllers(void)
{
#if DEBUG_INT_CODE
    bsp_printf("_bsp_install_cpu_irq_controllers()\n");
#endif /* DEBUG_INT_CODE */

#ifdef __CPU_SA1100__
    {
        extern void _bsp_install_sa1100_irq_controllers(void);
        _bsp_install_sa1100_irq_controllers();
    }
#endif /* __CPU_SA1100__ */

#ifdef __CPU_SA110__
    {
        extern void _bsp_install_sa110_irq_controllers(void);
        _bsp_install_sa110_irq_controllers();
    }
#endif /* __CPU_SA110__ */

#ifdef __XSCALE__
    {
        extern void _bsp_install_xscale_irq_controllers (void);
        _bsp_install_xscale_irq_controllers ();
    }
#endif /* __XSCALE__ */

#ifdef __CPU_LH77790A__
    {
        extern void _bsp_install_lh790_irq_controllers(void);
        _bsp_install_lh790_irq_controllers();
    }
#endif /* __CPU_LH77790A__ */

#if DEBUG_INT_CODE
    bsp_printf("Done w/ _bsp_install_cpu_irq_controllers()\n");
#endif /* DEBUG_INT_CODE */
}

/*
 * This is the generic exception handler that is called by all exceptions
 * after the regs structure is fixed up
 */
void generic_exception_handler(ex_regs_t *regs, unsigned long vect_num)
{
#if DEBUG_INT_CODE && !defined(NDEBUG)
    {
        union arm_psr cur_psr;
        union arm_insn inst;

        /*
         * What is the current psr
         */
        cur_psr.word = __get_cpsr();

        BSP_ASSERT(cur_psr.psr.mode == ARM_PSR_MODE_SVC);
        switch (vect_num)
        {
        case BSP_CORE_EXC_PREFETCH_ABORT:
        case BSP_CORE_EXC_DATA_ABORT:
            /*
             * No assertions
             */
            break;
        case BSP_CORE_EXC_SOFTWARE_INTERRUPT:
            /*
             * Verify that we were executing a swi instruction.
             */
            if (bsp_memory_read((void *)(regs->_lr - ARM_INST_SIZE), 0, 
                                ARM_INST_SIZE * 8, 1, &(inst.word)) != 0)
            {
                BSP_ASSERT(inst.swi.rsv1 == SWI_RSV1_VALUE);
            }
            break;

        case BSP_CORE_EXC_UNDEFINED_INSTRUCTION:
            /*
             * No assertions
             */
            break;

        case BSP_CORE_EXC_ADDRESS_ERROR_26_BIT:
            /*
             * No assertions
             */
            break;

        case BSP_CORE_EXC_IRQ:
            /*
             * Verify that IRQ's are masked
             */
            BSP_ASSERT(cur_psr.psr.i_bit = 1);
            break;

        case BSP_CORE_EXC_FIQ:
            /*
             * Verify that IRQ's and FIQ's are masked
             */
            BSP_ASSERT(cur_psr.psr.i_bit = 1);
            BSP_ASSERT(cur_psr.psr.f_bit = 1);
            break;

        default:
            bsp_printf("Unknown exception %d\n", vect_num);
            break;
        }
    }
#endif /* defined(DEBUG_INT_CODE) && !defined(NDEBUG) */

#if DEBUG_INT_CODE
    if ((vect_num != BSP_CORE_EXC_PREFETCH_ABORT) &&
        (vect_num != BSP_CORE_EXC_DATA_ABORT))
    {
        /*
         * Only print stuff if we are not processing
         * an abort exception already.  If we do a stack
         * dump in an exception, that will corrupt the 
         * jmpbuf already in use in generic_mem.c
         *
         * Besides, we really don't want to produce a
         * double-abort situation
         */
        bsp_printf("In generic_exception_handler()\n");
        exceptionPrint(regs, vect_num);
    }
#endif /* DEBUG_INT_CODE */

    /*
     * Call the dispatch routine to see if anyone
     * has registered for this exception.
     */
    if (_bsp_exc_dispatch(vect_num, regs) == 0)
    {
        union arm_insn inst;

        /*
         * If this is an illegal instruction, we may need to adjust
         * the PC if it is a breakpoint that GDB inserted.
         */
        if ((vect_num == BSP_CORE_EXC_UNDEFINED_INSTRUCTION) &&
            (bsp_memory_read((void *)(regs->_pc - ARM_INST_SIZE), 0, 
                             ARM_INST_SIZE * 8, 1, &(inst.word)) != 0))
        {
            /*
             * We were able to read this address. It must be a valid address.
             */
            if (inst.word == BREAKPOINT_INSN)
            {
                /*
                 * Adjust the PC to point at the breakpoint instruction
                 * itself.
                 */
                regs->_pc -= ARM_INST_SIZE;
            }
        }
         
	
         /*
          * nobody handled this exception.
          * Let's let GDB take care of it
          */
         bsp_invoke_dbg_handler(vect_num, regs);
    }

#if DEBUG_INT_CODE
    if ((vect_num != BSP_CORE_EXC_PREFETCH_ABORT) &&
        (vect_num != BSP_CORE_EXC_DATA_ABORT))
    {
        /*
         * Only print stuff if we are not processing
         * an abort exception already.  If we do a stack
         * dump in an exception, that will corrupt the 
         * jmpbuf already in use in generic_mem.c
         *
         * Besides, we really don't want to produce a
         * double-abort situation
         */
        bsp_printf("Returning from exception\n");
        _bsp_dump_regs(regs);
    }
#endif /* DEBUG_INT_CODE */
}
