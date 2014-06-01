/*
 * cpu.c -- SPARC specific support for Cygnus BSP
 *
 * Copyright (c) 1999 Cygnus Support
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

#ifdef __CPU_MB8683X__
#include "mb86940.h"
#endif /* __CPU_MB8683X__ */

#ifdef BSP_LOG
#include "bsplog.h"
#endif

void
_bsp_dump_regs(void *regs)
{
    ex_regs_t *p = regs;

    bsp_printf(" g0[0x%08lx]  g1[0x%08lx]  g2[0x%08lx]  g3[0x%08lx]\n",
               p->_g0, p->_g1, p->_g2, p->_g3);
    bsp_printf(" g4[0x%08lx]  g5[0x%08lx]  g6[0x%08lx]  g7[0x%08lx]\n",
               p->_g4, p->_g5, p->_g6, p->_g7);
    bsp_printf(" o0[0x%08lx]  o1[0x%08lx]  o2[0x%08lx]  o3[0x%08lx]\n",
               p->_o0, p->_o1, p->_o2, p->_o3);
    bsp_printf(" o4[0x%08lx]  o5[0x%08lx]  sp[0x%08lx]  o7[0x%08lx]\n",
               p->_o4, p->_o5, p->_sp, p->_o7);
    bsp_printf(" l0[0x%08lx]  l1[0x%08lx]  l2[0x%08lx]  l3[0x%08lx]\n",
               p->_l0, p->_l1, p->_l2, p->_l3);
    bsp_printf(" l4[0x%08lx]  l5[0x%08lx]  l6[0x%08lx]  l7[0x%08lx]\n",
               p->_l4, p->_l5, p->_l6, p->_l7);
    bsp_printf(" i0[0x%08lx]  i1[0x%08lx]  i2[0x%08lx]  i3[0x%08lx]\n",
               p->_i0, p->_i1, p->_i2, p->_i3);
    bsp_printf(" i4[0x%08lx]  i5[0x%08lx]  fp[0x%08lx]  i7[0x%08lx]\n",
               p->_i4, p->_i5, p->_fp, p->_i7);
    bsp_printf("  y[0x%08lx] psr[0x%08x]  wim[0x%08lx] tbr[0x%08lx]\n",
               p->_y, p->_psr, p->_wim, p->_tbr);
    bsp_printf(" pc[0x%08lx] npc[0x%08x]  fps[0x%08lx] cps[0x%08lx]\n",
               p->_pc, p->_npc, p->_fpsr, p->_cpsr);
    bsp_printf("dia1[0x%08lx] dia2[0x%08x] dda1[0x%08lx] dda2[0x%08lx]\n",
               p->_dia1, p->_dia2, p->_dda1, p->_dda2);
    bsp_printf("div1[0x%08lx] div2[0x%08x] dcr[0x%08lx] dsr[0x%08lx]\n",
               p->_ddv1, p->_ddv2, p->_dcr, p->_dsr);
}

#define DEBUG_SYSCALL 0

static int
syscall_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int err, func;
    int arg1, arg2, arg3, arg4;

    func = regs->_o0;
    arg1 = regs->_o1;
    arg2 = regs->_o2;
    arg3 = regs->_o3;
    arg4 = regs->_o4;

#if DEBUG_SYSCALL
    bsp_printf("handle_syscall: func<%d> args<0x%x, 0x%x, 0x%x, 0x%x>...\n",
               func, arg1, arg2, arg3, arg4);
#endif

    /*
     * skip past trap opcode.
     */
    regs->_pc  += 4;
    regs->_npc += 4;

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
        (*bsp_shared_data->__dbg_vector)(BSP_EXC_INT1, regs);

        return 1;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
        regs->_o0 = err;
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


void
_bsp_install_vsr(void)
{
    extern void _bsp_default_handler(void);
    extern void _bsp_nmi_handler(void);
    extern void _bsp_wovf_handler(void);
    extern void _bsp_wund_handler(void);
    extern void _bsp_wflush_handler(void);
    extern void _bsp_stub_entry(void);
    extern void *bsp_vsr_table[];
    int  i;
    void **p = bsp_vsr_table;

    for (i = 0; i < BSP_MAX_EXCEPTIONS; i++)
        *p++ = _bsp_default_handler;

    bsp_vsr_table[BSP_EXC_INT15] = _bsp_nmi_handler;
    bsp_vsr_table[BSP_EXC_WINOVF] = _bsp_wovf_handler;
    bsp_vsr_table[BSP_EXC_WINUND] = _bsp_wund_handler;
    bsp_vsr_table[BSP_EXC_WINFLUSH] = _bsp_wflush_handler;
    bsp_vsr_table[BSP_VEC_BSPDATA] = bsp_shared_data;
    bsp_vsr_table[BSP_VEC_STUB_ENTRY] = _bsp_stub_entry;
    bsp_vsr_table[BSP_VEC_MT_DEBUG] = NULL;
    bsp_flush_dcache(bsp_vsr_table, sizeof(void*) * NUM_VTAB_ENTRIES);
}

#ifdef BSP_LOG
int bsp_dump_flag;

static int
nmi_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;

    bsp_log("NMI: pc[0x%x] sp[0x%x]\n", regs->_pc, regs->_sp);
    bsp_log_dump();
    while (1);
    return 0;
}
#endif /* BSP_LOG */


/*
 *  Architecture specific BSP initialization.
 */
void
_bsp_cpu_init(void)
{
    static bsp_vec_t sc_vec;
#ifdef BSP_LOG
    static bsp_vec_t nmi_vec;
#endif

    _bsp_install_vsr();

    bsp_shared_data->__flush_dcache = __dcache_flush;
    bsp_shared_data->__flush_icache = __icache_flush;

    /*
     * Install syscall handler.
     */
    sc_vec.handler = syscall_handler;
    sc_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_SYSCALL,
                    BSP_VEC_REPLACE, &sc_vec);
#ifdef BSP_LOG
    nmi_vec.handler = nmi_handler;
    nmi_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_INT15,
                    BSP_VEC_REPLACE, &nmi_vec);

    /* put log in SRAM */
    bsp_log_init((int *)0x30000000, 128 * 1024, bsp_dump_flag);
#endif
}
