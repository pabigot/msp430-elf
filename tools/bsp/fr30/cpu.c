/*
 * cpu.c -- FR30 specific support for Cygnus BSP
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

#ifdef BSP_LOG
#include "bsplog.h"
#endif

void
_bsp_dump_regs(void *regs)
{
    ex_regs_t *p = regs;

    bsp_printf(" r0[0x%08lx]  r1[0x%08lx]  r2[0x%08lx]  r3[0x%08lx]\n",
		p->_r0, p->_r1, p->_r2, p->_r3);
    bsp_printf(" r4[0x%08lx]  r5[0x%08lx]  r6[0x%08lx]  r7[0x%08lx]\n",
		p->_r4, p->_r5, p->_r6, p->_r7);
    bsp_printf(" r8[0x%08lx]  r9[0x%08lx] r10[0x%08lx] r11[0x%08lx]\n",
		p->_r8, p->_r9, p->_r10, p->_r11);
    bsp_printf("r12[0x%08lx] r13[0x%08lx] r14[0x%08lx] r15[0x%08lx]\n",
		p->_r12, p->_r13, p->_r14, p->_r15);

    bsp_printf(" pc[0x%08lx] ps[0x%08lx]  tbr[0x%08lx] rp[0x%08lx]\n",
		p->_pc, p->_ps, p->_tbr, p->_rp);
    bsp_printf(" ssp[0x%08lx] usp[0x%08lx]  mdh[0x%08lx] mdl[0x%08lx]\n",
		p->_ssp, p->_usp, p->_mdh, p->_mdl);
}

#define DEBUG_SYSCALL 0

static int
syscall_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int err, func;
    int arg1, arg2, arg3, arg4;

    func = regs->_r4;
    arg1 = regs->_r5;
    arg2 = regs->_r6;
    arg3 = regs->_r7;
    arg4 = regs->_r8;  /* !!FIXME!! */

#if DEBUG_SYSCALL
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
        (*bsp_shared_data->__dbg_vector)(BSP_EXC_NMI, regs);

        return 1;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
	regs->_r4 = err;
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


extern void __icache_init(void);
extern void __icache_enable(void);
extern void __icache_flush(void *addr, int len);

#ifdef BSP_LOG
static int
nmi_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;

    bsp_log("NMI: pc[0x%x] sp[0x%x]\n", regs->_pc, regs->_r15);
    bsp_log_dump();
    while (1);
    return 0;
}
#endif

/*
 *  Architecture specific BSP initialization.
 */
void
_bsp_cpu_init(void)
{
    static bsp_vec_t sc_vec;
#ifdef BSP_LOG
    static bsp_vec_t nmi_vec;
    extern int bsp_dump_flag;
#endif

    __icache_init();
    __icache_enable();
    bsp_shared_data->__flush_icache = __icache_flush;

    /*
     * Install syscall handler.
     */
    sc_vec.handler = syscall_handler;
    sc_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_INT10,
		    BSP_VEC_REPLACE, &sc_vec);

#ifdef BSP_LOG
    nmi_vec.handler = nmi_handler;
    nmi_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_NMI,
		    BSP_VEC_REPLACE, &nmi_vec);

    bsp_log_init((int *)0x600000, 512 * 1024, bsp_dump_flag);
#endif
}
