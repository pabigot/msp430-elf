/*
 * cpu.c -- M68K specific support for Cygnus BSP
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
#include "syscall.h"
#include <stdlib.h>

#define USE_SAMPLE_BUS_ADDR_HANDLERS 0

void
_bsp_dump_regs(void *regs)
{
    ex_regs_t *p = regs;

    bsp_printf(" d0[0x%08lx]  d1[0x%08lx]  d2[0x%08lx]  d3[0x%08lx]\n", 
               p->_d0, p->_d1, p->_d2, p->_d3);
    bsp_printf(" d4[0x%08lx]  d5[0x%08lx]  d6[0x%08lx]  d7[0x%08lx]\n",
               p->_d4, p->_d5, p->_d6, p->_d7);
    bsp_printf(" a0[0x%08lx]  a1[0x%08lx]  a2[0x%08lx]  a3[0x%08lx]\n", 
               p->_a0, p->_a1, p->_a2, p->_a3);
    bsp_printf(" a4[0x%08lx]  a5[0x%08lx]  a6[0x%08lx]  a7[0x%08lx]\n",
               p->_a4, p->_a5, p->_a6, p->_a7);
    bsp_printf(" sr[0x%04lx]      pc[0x%08lx]\n",
               p->_sr, p->_pc);
}

#define DEBUG_SYSCALL 0

static int
syscall_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int func, arg1, arg2, arg3, arg4;
    int err;

    func = *((unsigned int *)(regs->_sp + 4));
    arg1 = *((unsigned int *)(regs->_sp + 8));
    arg2 = *((unsigned int *)(regs->_sp + 12));
    arg3 = *((unsigned int *)(regs->_sp + 16));
    arg4 = *((unsigned int *)(regs->_sp + 20));

#if DEBUG_SYSCALL
    bsp_printf("regs_ptr = 0x%08lx\n", regs_ptr);
    bsp_printf("regs->_sp = 0x%08lx\n", regs->_sp);
    bsp_printf("handle_syscall: func<%d> args<0x%x, 0x%x, 0x%x, 0x%x>...\n",
               func, arg1, arg2, arg3, arg4);
#endif

    if (func == SYS_exit) {
        /*
         *  We want to stop in exit so that the user may poke around
         *  to see why his app exited.
         */
        (*bsp_shared_data->__dbg_vector)(exc_nr, regs);

        return 1;
    }

    if (func == SYS_interrupt) {
        /*
         *  A console interrupt landed us here.
	 *  Invoke the debug agent so as to cause a SIGINT.
         */
        (*bsp_shared_data->__dbg_vector)(BSP_CORE_EXC_LEVEL_7_AUTO, regs);

        return 1;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
        regs->_d0 = err;
#if DEBUG_SYSCALL
        bsp_printf("returned: %d\n",err);
#endif
        return 1;
    }

#if DEBUG_SYSCALL
    bsp_printf("not handled.\n");
#endif

    return 0;
}

#if USE_SAMPLE_BUS_ADDR_HANDLERS
static int addr_err_handler(int ex_nr, ex_regs_t *regs);
static int bus_err_handler(int ex_nr, ex_regs_t *regs);
#endif /* USE_SAMPLE_BUS_ADDR_HANDLERS */

/*
 *  Architecture specific BSP initialization.
 */
void
_bsp_cpu_init(void)
{
    static bsp_vec_t sc_vec;

    /*
     * Install syscall handler.
     */
    sc_vec.handler = syscall_handler;
    sc_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_TRAP_15,
                    BSP_VEC_REPLACE, &sc_vec);

#if USE_SAMPLE_BUS_ADDR_HANDLERS
    /*
     * Setup default handlers for address and bus errors
     */
    {
        static bsp_vec_t addr_irq_vec;
        static bsp_vec_t bus_irq_vec;

        addr_irq_vec.handler = (void *)addr_err_handler;
        addr_irq_vec.next = NULL;
        bus_irq_vec.handler = (void *)bus_err_handler;
        bus_irq_vec.next = NULL;

        bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_ADDR_ERROR, BSP_VEC_CHAIN_LAST, &addr_irq_vec);
        bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_BUS_ERROR, BSP_VEC_CHAIN_LAST, &bus_irq_vec);
    }
#endif
}

#if USE_SAMPLE_BUS_ADDR_HANDLERS

static int
addr_err_handler(int ex_nr, ex_regs_t *regs)
{
    /*
     * The stack frame is calculated based on
     * the stack that is setup by the code in
     * start.S.  See that file for a stack
     * diagram.
   */
    m68k_bus_addr_stkfrm *frame = (m68k_bus_addr_stkfrm *)((unsigned long)regs
                                                           + (unsigned long)sizeof(*regs)
                                                           + (unsigned long)sizeof(ex_nr));

    bsp_printf("Address Error\n");
    bsp_printf("Register Dump (0x%08lx):\n", (unsigned long)regs);
    _bsp_dump_regs(regs);
    bsp_printf("Stack frame (0x%08lx):\n", frame);
    bsp_printf("\tcode = 0x%04x\taccess address = 0x%04x%04x\n", 
               frame->code, frame->access_h, frame->access_l);
    bsp_printf("\tir   = 0x%04x\tsr = 0x%04x\n", frame->ir, frame->sr);
    bsp_printf("\tpc   = 0x%04x%04x\n", frame->pc_h, frame->pc_l);

    while (1) ;
    return 1;
}

static int
bus_err_handler(int ex_nr, ex_regs_t *regs)
{
    /*
     * The stack frame is calculated based on
     * the stack that is setup by the code in
     * start.S.  See that file for a stack
     * diagram.
   */
    m68k_bus_addr_stkfrm *frame = (m68k_bus_addr_stkfrm *)((unsigned long)regs
                                                           + (unsigned long)sizeof(*regs)
                                                           + (unsigned long)sizeof(ex_nr));

    bsp_printf("Bus Error\n");
    bsp_printf("Register Dump (0x%08lx):\n", (unsigned long)regs);
    _bsp_dump_regs(regs);
    bsp_printf("Stack frame (0x%08lx):\n", frame);
    bsp_printf("\tcode = 0x%04x\taccess address = 0x%04x%04x\n", 
               frame->code, frame->access_h, frame->access_l);
    bsp_printf("\tir   = 0x%04x\tsr = 0x%04x\n", frame->ir, frame->sr);
    bsp_printf("\tpc   = 0x%04x%04x\n", frame->pc_h, frame->pc_l);

    while (1) ;
    return 1;
}


int generate_bus_addr_error(void)
{
#if 0
    /*
     * Force Address Error
     */
    *(unsigned long*)0x3 = 4;
#else
    /*
     * Force Bus Error
     */
    *(unsigned long*)0x600000 = 4;
#endif 

    return 0;
}
#endif /* USE_SAMPLE_BUS_ADDR_HANDLERS */
