/*
 * cpu.c -- CPU specific support for Cygnus BSP
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

/*
 *  Print out register contents.
 */
void
_bsp_dump_regs(void *regs)
{
    ex_regs_t *p = regs;

    bsp_printf(" a0[0x%08lx]  a1[0x%08lx]  a2[0x%08lx]  a3[0x%08lx]\n",
	       p->_a0, p->_a1, p->_a2, p->_a3);
    bsp_printf(" d0[0x%08lx]  d1[0x%08lx]  d2[0x%08lx]  d3[0x%08lx]\n",
	       p->_d0, p->_d1, p->_d2, p->_d3);
    bsp_printf(" pc[0x%08lx] sp[0x%08lx]  psw[0x%08lx]\n",
	       p->_pc, p->_sp, p->_psw);
    bsp_printf(" lar[0x%08lx] lir[0x%08lx]  mdr[0x%08lx]\n",
	       p->_lar, p->_lir, p->_mdr);
}


/*
 *  IRQ_SYSERR handler for NMI caused by syscall instruction.
 */
static int
_syserr_syscall(int nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int err, func;
    int arg1, arg2, arg3, arg4;

    /*
     *  Check for syscall opcode.
     */
    func = regs->_d0;
    arg1 = regs->_d1;
    arg2 = *((unsigned int *)(regs->_sp + 12));
    arg3 = *((unsigned int *)(regs->_sp + 16));
    arg4 = *((unsigned int *)(regs->_sp + 20));

#ifdef DEBUG_SYSCALL
    bsp_printf("handle_syscall: func<%d> args<0x%x, 0x%x, 0x%x, 0x%x>...",
	       func, arg1, arg2, arg3, arg4);
#endif

    /*
     * skip past 'syscall' opcode.
     */
    regs->_pc += 2;

    if (func == SYS_exit) {
	/*
	 *  We want to stop in exit so that the user may poke around
	 *  to see why his app exited.
	 */
	(*bsp_shared_data->__dbg_vector)(nr, regs);
	return 1;
    }

    if (func == SYS_interrupt) {
        /*
         *  A console interrupt landed us here.
	 *  Invoke the debug agent so as to cause a SIGINT.
         */
	regs->_dummy = /*TARGET_SIGNAL_INT*/ 2;
        (*bsp_shared_data->__dbg_vector)(BSP_EXC_IRQ, regs);

        return 1;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
	regs->_d0 = err;
#ifdef DEBUG_SYSCALL
	bsp_printf("returned: %d\n",err);
#endif
	return 1;
    }

    return 0;
}



/*
 *  Late initialization.
 */
void
_bsp_cpu_init(void)
{
    static bsp_vec_t sc_vec;

    sc_vec.handler = _syserr_syscall;
    sc_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_INTERRUPT, BSP_EXC_SYSCALL,
		    BSP_VEC_CHAIN_LAST, &sc_vec);
}

