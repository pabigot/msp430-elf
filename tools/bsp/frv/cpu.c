/*
 * cpu.c -- FR-V specific support for Cygnus BSP
 *
 * Copyright (c) 1999, 2000 Cygnus Support
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
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include "bsp_if.h"
#include "syscall.h"
#include "fr500.h"

void
_bsp_dump_regs(void *saved_regs)
{
    ex_regs_t *regs = saved_regs;
    unsigned *p;
    int i;

    p = &regs->_gpr[0];
    for (i = 0; i < 64; i++) {
	bsp_printf("%sgr%d[%08x] ", (i<10)?" ":"", i, *p);
	++p;
	if ((i & 3) == 3)
	    bsp_printf("\n");
    }

    p = (unsigned *)&regs->_fpr[0];
    for (i = 0; i < 64; i++) {
	bsp_printf("%sfr%d[%08x] ", (i<10)?" ":"", i, *p);
	++p;
	if ((i & 3) == 3)
	    bsp_printf("\n");
    }

    bsp_printf("pc[%08x]  psr[%08x] lr[%08x] lcr[%08x]\n",
	       regs->_pc, regs->_psr, regs->_lr, regs->_lcr);
}


#define DEBUG_SYSCALL 0

static int
syscall_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int err, func;
    int arg1, arg2, arg3, arg4;

    func = regs->_gpr[8];
    arg1 = regs->_gpr[9];
    arg2 = regs->_gpr[10];
    arg3 = regs->_gpr[11];
    arg4 = regs->_gpr[12];

#if DEBUG_SYSCALL
    bsp_printf("handle_syscall: func<%d> args<0x%x, 0x%x, 0x%x, 0x%x>...\n",
               func, arg1, arg2, arg3, arg4);
#endif

    if (func == SYS_exit || func == SYS_interrupt) {
        /*
         *  We want to stop in exit so that the user may poke around
         *  to see why his app exited. We also want to stop on
	 *  SYS_interrupt but with SIGINT, not SIGTRAP.
	 *
	 *  The low-level code in _bsp_stub_low will detect these
	 *  two events and act accordingly. Return zero so the stub
	 *  will get invoked.
         */
        return 0;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
        regs->_gpr[8] = err;
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

extern void __break_switch(void);
volatile int __switch_trapnum;

void
_bsp_break_handler(ex_regs_t *regs)
{
    bsp_handler_t handler;
    unsigned brr;
    int tt;

    /* get and clear brr bits */
    regs->_brr = brr = __get_brr();
    __set_brr(brr);

#if DEBUG_BREAK
    bsp_printf("_break: brr[%08x] bpcsr[%08x] pcsr[%x] tt[%x] psr[%x]\n",
	       brr, regs->_bpcsr, regs->_pcsr, __switch_trapnum, __get_psr());
#endif
    
    if (brr & BRR_SB) {
	/* break insn */
	regs->_bpcsr -= 4;
	regs->_pc -= 4;
    }

    if (regs->_bpcsr == (unsigned)__break_switch) {
	/* forced break after normal-mode exception */
	regs->_bpcsr = regs->_pc = regs->_pcsr;
	regs->_bpsr |= 1; /* always set ET upon return */
	if (regs->_psr & PSR_PS)
	    regs->_bpsr |= (1 << 12);
	else
	    regs->_bpsr &= ~(1 << 12);

	if ((tt = __switch_trapnum) == 0x80) {
	    /* its a syscall */
	    if (regs->_gpr[8] == SYS_interrupt)
		tt = TT_INT1;
	    if (regs->_gpr[8] == SYS_interrupt || regs->_gpr[8] == SYS_exit)
		regs->_pc = regs->_bpcsr = regs->_lr;
	}

    } else if (brr & BRR_EB)
	tt = ((brr >> BRR_EBTT_SHIFT) & BRR_EBTT_MASK);
    else
	tt = 0xff;

    bsp_invoke_dbg_handler(tt, regs);
}


/*
 * Install vectors into VSR table.
 */
void
_bsp_install_vsr(void)
{
    extern void _bsp_default_handler(void);
    extern void _bsp_stub_low(void);
    extern void *bsp_vsr_table[];
    int  i;
    void **p = bsp_vsr_table;

    for (i = 0; i < NUM_VTAB_ENTRIES; i++)
	*p++ = _bsp_default_handler;

    bsp_vsr_table[BSP_VEC_STUB_ENTRY] = _bsp_stub_low;
    bsp_vsr_table[BSP_VEC_BSPDATA] = bsp_shared_data;
    bsp_vsr_table[BSP_VEC_MT_DEBUG] = NULL;
}


/*
 *  Architecture specific BSP initialization.
 */
void
_bsp_cpu_init(void)
{
    extern void __dcache_flush(void *addr, int nbytes);
    extern void __icache_flush(void *addr, int nbytes);
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

