/*
 * cpu.c -- MIPS specific support for Cygnus BSP
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
 */

#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "insn.h"
#include "bsp_if.h"
#include "syscall.h"
#include <stdlib.h>


void
_bsp_dump_regs(void *regs)
{
    ex_regs_t *p = regs;

    bsp_printf(" r0[0x%08lx]  at[0x%08lx]  v0[0x%08lx]  v1[0x%08lx]\n",
               p->_zero, p->_at, p->_v0, p->_v1);
    bsp_printf(" a0[0x%08lx]  a1[0x%08lx]  a2[0x%08lx]  a3[0x%08lx]\n",
               p->_a0, p->_a1, p->_a2, p->_a3);
    bsp_printf(" t0[0x%08lx]  t1[0x%08lx]  t2[0x%08lx]  t3[0x%08lx]\n",
               p->_t0, p->_t1, p->_t2, p->_t3);
    bsp_printf(" t4[0x%08lx]  t5[0x%08lx]  t6[0x%08lx]  t7[0x%08lx]\n",
               p->_t4, p->_t5, p->_t6, p->_t7);
    bsp_printf(" s0[0x%08lx]  s1[0x%08lx]  s2[0x%08lx]  s3[0x%08lx]\n",
               p->_s0, p->_s1, p->_s2, p->_s3);
    bsp_printf(" s4[0x%08lx]  s5[0x%08lx]  s6[0x%08lx]  s7[0x%08lx]\n",
               p->_s4, p->_s5, p->_s6, p->_s7);
    bsp_printf(" t8[0x%08lx]  t9[0x%08lx]  k0[0x%08lx]  k1[0x%08lx]\n",
               p->_t8, p->_t9, p->_k0, p->_k1);
    bsp_printf(" gp[0x%08lx]  sp[0x%08lx]  fp[0x%08lx]  ra[0x%08lx]\n",
	       p->_gp, p->_sp, p->_fp, p->_ra);
    bsp_printf(" pc[0x%08lx]  sr[0x%08x]  hi[0x%08lx]  lo[0x%08lx]\n",
	       p->_pc, (unsigned)(p->_sr & 0xffffffff), p->_hi, p->_lo);
    bsp_printf(" cause[0x%08x]  badvaddr[0x%08lx]\n",
	       (unsigned)(p->_cause & 0xffffffff), p->_bad);
}

#define DEBUG_SYSCALL 0

static int
syscall_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int err, func;
    int arg1, arg2, arg3, arg4;

    func = regs->_a0;
    arg1 = regs->_a1;
    arg2 = regs->_a2;
    arg3 = regs->_a3;
    arg4 = regs->_t0;

#if DEBUG_SYSCALL
    bsp_printf("handle_syscall: func<%d> args<0x%x, 0x%x, 0x%x, 0x%x>...\n",
               func, arg1, arg2, arg3, arg4);
    bsp_printf("Register Dump (0x%08lx):\n", (unsigned long)regs);
    _bsp_dump_regs(regs);
#endif

    /*
     * skip past 'syscall' opcode.
     */
    bsp_set_pc(bsp_get_pc(regs) + 4, regs);

    if (func == SYS_exit || func == SYS_interrupt) {
        /*
         *  We want to stop in exit so that the user may poke around
         *  to see why his app exited. We also want to stop on
	 *  SYS_interrupt but with SIGINT, not SIGTRAP.
	 *
	 *  The low-level code in _bsp_stub_entry will detect these
	 *  two events and act accordingly. Return zero so the stub
	 *  will get invoked.
         */
        return 0;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
        regs->_v0 = err;
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


#if NUM_TLB_ENTRIES
void
__tlb_init(void)
{
    int i;

#ifdef __R3000
    unsigned long  entryhi;

    asm volatile ("\tmtc0  $0,$2\n"  /* entrylo */
		  : : );
    entryhi = 0xa0000000L;
#else
    unsigned long long entryhi;

    asm volatile ("\tmtc0  $0,$5\n"  /* 4K pages */
		  "\tmtc0  $0,$2\n"  /* entrylo0 */
		  "\tmtc0  $0,$3\n"  /* entrylo1 */
		  : : );
    entryhi = 0xbfffff00a0000000L;
#endif

    for (i = 0; i < NUM_TLB_ENTRIES; i++) {
	asm volatile ("\tmtc0   %0,$0\n"   /* set index register */
		      "\tnop\n"
		      "\tmtc0   %1,$10\n"  /* set entryhi */
		      "\tnop\n"            /* pipeline delay */
		      "\tnop\n"
		      "\tnop\n"
		      "\tnop\n"
		      "\tnop\n"
		      "\ttlbwi\n"
		      "\tnop\n"            /* pipeline delay */
		      "\tnop\n"
		      : /* no outputs */
		      : "r" (i), "r" (entryhi) );
	entryhi += 4096;
#ifndef __R3000
	entryhi += 4096;
#endif
    }
}
#endif


/*
 * Install vectors into VSR table.
 */
void
_bsp_install_vsr(void)
{
    extern int  _bsp_check_unexpected_irq(int irq_nr);
    extern void _bsp_default_handler(void);
    extern void _bsp_stub_entry(void);
    extern void __install_async_breakpoint(void *epc);
    extern void *bsp_vsr_table[];

    int  i, *ip;
    void **p = bsp_vsr_table;

    for (i = 0; i < NUM_VTAB_ENTRIES; i++)
	*p++ = _bsp_default_handler;

    bsp_vsr_table[BSP_VEC_BSPDATA] = bsp_shared_data;
    bsp_vsr_table[BSP_VEC_BP_HOOK] = __install_async_breakpoint;
    bsp_vsr_table[BSP_VEC_MT_DEBUG] = NULL;
    bsp_vsr_table[BSP_VEC_STUB_ENTRY] = _bsp_stub_entry;
    bsp_vsr_table[BSP_VEC_IRQ_CHECK] = _bsp_check_unexpected_irq;

    ip = (int *)&bsp_vsr_table[BSP_VEC_MAGIC];
    *ip = 0x55aa4321;

    bsp_flush_dcache(bsp_vsr_table, sizeof(void*) * NUM_VTAB_ENTRIES);
}


/*
 *  Architecture specific BSP initialization.
 */
void
_bsp_cpu_init(void)
{
    static bsp_vec_t sc_vec;

    bsp_shared_data->__flush_dcache = __dcache_flush;
    bsp_shared_data->__flush_icache = __icache_flush;

    _bsp_install_vsr();

    /*
     * Install syscall handler.
     */
    sc_vec.handler = syscall_handler;
    sc_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_SYSCALL,
                    BSP_VEC_REPLACE, &sc_vec);
}

