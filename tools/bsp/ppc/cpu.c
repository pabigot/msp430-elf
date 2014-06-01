/*
 * cpu.c -- PPC specific support for Cygnus BSP
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

void
_bsp_dump_regs(void *regs)
{
    ex_regs_t *p = regs;

    bsp_printf(" r0[0x%08lx]  r1[0x%08lx]  r2[0x%08lx]  r3[0x%08lx]\n",
		p->_gpr[0], p->_gpr[1], p->_gpr[2], p->_gpr[3]);
    bsp_printf(" r4[0x%08lx]  r5[0x%08lx]  r6[0x%08lx]  r7[0x%08lx]\n",
		p->_gpr[4], p->_gpr[5], p->_gpr[6], p->_gpr[7]);
    bsp_printf(" r8[0x%08lx]  r9[0x%08lx] r10[0x%08lx] r11[0x%08lx]\n",
		p->_gpr[8], p->_gpr[9], p->_gpr[10], p->_gpr[11]);
    bsp_printf("r12[0x%08lx] r13[0x%08lx] r14[0x%08lx] r15[0x%08lx]\n",
		p->_gpr[12], p->_gpr[13], p->_gpr[14], p->_gpr[15]);
    bsp_printf("r16[0x%08lx] r17[0x%08lx] r18[0x%08lx] r19[0x%08lx]\n",
		p->_gpr[16], p->_gpr[17], p->_gpr[18], p->_gpr[19]);
    bsp_printf("r20[0x%08lx] r21[0x%08lx] r22[0x%08lx] r23[0x%08lx]\n",
		p->_gpr[20], p->_gpr[21], p->_gpr[22], p->_gpr[23]);
    bsp_printf("r24[0x%08lx] r25[0x%08lx] r26[0x%08lx] r27[0x%08lx]\n",
		p->_gpr[24], p->_gpr[25], p->_gpr[26], p->_gpr[27]);
    bsp_printf("r28[0x%08lx] r29[0x%08lx] r30[0x%08lx] r31[0x%08lx]\n",
		p->_gpr[28], p->_gpr[29], p->_gpr[30], p->_gpr[31]);

    bsp_printf(" pc[0x%08lx] msr[0x%08lx]  cr[0x%08lx]\n lr[0x%08lx] ctr[0x%08lx] xer[0x%08lx]\n",
		p->_nip, p->_msr, p->_ccr, p->_lr, p->_cnt, p->_xer);
}



static int
syscall_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int err, func;
    int arg1, arg2, arg3, arg4;

    func = regs->_gpr[3];
    arg1 = regs->_gpr[4];
    arg2 = regs->_gpr[5];
    arg3 = regs->_gpr[6];
    arg4 = regs->_gpr[7];

#ifdef DEBUG_SYSCALL
    bsp_printf("handle_syscall: func<%d> args<0x%x, 0x%x, 0x%x, 0x%x>...",
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
        (*bsp_shared_data->__dbg_vector)(BSP_EXC_EXTIRQ, regs);

        return 1;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
	regs->_gpr[3] = err;
#ifdef DEBUG_SYSCALL
	bsp_printf("returned: %d\n",err);
#endif
	return 1;
    }

#ifdef DEBUG_SYSCALL
    bsp_printf("not handled.\n");
#endif

    return 0;
}

unsigned int  __decr_cnt;

static unsigned long ticks;
static unsigned long last_count;

unsigned long
_bsp_ms_ticks(void)
{
    unsigned long current = __get_decr();
    static unsigned long diff;

    /* its a down counter, so use "last - current" */
    diff += last_count - current;

    if (diff >= __decr_cnt) {
	ticks += (diff / __decr_cnt);
	ticks = 0;
    }

    last_count = current;

    return ticks;
}


/*
 *  Default handler for decrementer exception.
 */
static int
decr_handler(int exc_nr, void *regs)
{
    /*
     * Just write current value back to clear interrupt.
     */
    asm volatile (
        "mfdec   3\n"
        "mtdec   3\n"
	: /* no outputs */
	: /* no inputs */  );

    _bsp_ms_ticks();

    return 1;
}


/*
 *  Architecture specific BSP initialization.
 */
void
_bsp_cpu_init(void)
{
    static bsp_vec_t sc_vec;
    static bsp_vec_t decr_vec;

#if defined(__CPU_MPC8XX__)

#if USE_DCACHE
    __dcache_enable();
    bsp_shared_data->__flush_dcache = __dcache_flush;
#endif

#if USE_ICACHE
    __icache_enable();
    bsp_shared_data->__flush_icache = __icache_flush;
#endif

#endif

    /*
     * Install syscall handler.
     */
    sc_vec.handler = syscall_handler;
    sc_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_SYSCALL,
		    BSP_VEC_REPLACE, &sc_vec);

    /*
     * Install decrementer handler.
     */
    decr_vec.handler = decr_handler;
    decr_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_EXC_DECR,
		    BSP_VEC_REPLACE, &decr_vec);

    bsp_enable_irq(BSP_IRQ_CORE);
#if defined(__CPU_MPC8XX__)
    bsp_enable_irq(BSP_IRQ_CPM);
#endif

#if defined(__CPU_PPC4XX__)
    {
	unsigned u = __get_msr();

	u |= (MSR_CE | MSR_ME | MSR_DE);
	__set_msr(u);
    }
    
    bsp_shared_data->__flush_dcache = __dcache_flush;
    bsp_shared_data->__flush_icache = __icache_flush;
#endif
}

