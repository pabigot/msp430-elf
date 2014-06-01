/*
 * cpu.c -- ARM(R) specific support for CygMon BSP
 *
 * Copyright (c) 1998, 1999, 2000 Red Hat, Inc.
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
 * Intel is a Registered Trademark of Intel Corporation.
 * StrongARM is a Registered Trademark of Advanced RISC Machines Limited.
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "insn.h"
#include "bsp_if.h"
#include "syscall.h"
#include <stdlib.h>

/*
 * CPU specific data for ARM(R) boards.
 */
#ifdef __CPU_SHARP_LH77790A__
#  include <lh77790a.h>
   static arm_cpu_data cpu_data = { 0 };
#endif /* __CPU_SHARP_LH77790A__ */

#ifdef __CPU_SA1100__
#   include <bsp/sa-1100.h>
#endif /* __CPU_SA1100__ */

#ifdef __CPU_SA110__
#   include <bsp/sa-110.h>
#endif /* __CPU_SA110__ */

void
_bsp_dump_regs(void *regs)
{
    ex_regs_t *p = regs;

    bsp_printf(" r0[0x%08lx]  r1[0x%08lx]    r2[0x%08lx]   r3[0x%08lx]\n",
               p->_r0, p->_r1, p->_r2, p->_r3);
    bsp_printf(" r4[0x%08lx]  r5[0x%08lx]    r6[0x%08lx]   r7[0x%08lx]\n",
               p->_r4, p->_r5, p->_r6, p->_r7);
    bsp_printf(" r8[0x%08lx]  r9[0x%08lx]    r10[0x%08lx]  r11[0x%08lx]\n",
               p->_r8, p->_r9, p->_r10, p->_r11);
    bsp_printf(" r12[0x%08lx] sp[0x%08lx]    lr[0x%08lx]\n",
               p->_r12, p->_sp, p->_lr);
    bsp_printf(" pc[0x%08lx]  cpsr[0x%08lx]  spsvc[0x%08lx]\n",
               p->_pc, p->_cpsr, p->_spsvc);
}

#define DEBUG_SYSCALL 0

static int
syscall_handler(int exc_nr, void *regs_ptr)
{
    ex_regs_t *regs = regs_ptr;
    int func, arg1, arg2, arg3, arg4;
    int err;
    union arm_insn inst;

    /*
     * What is the instruction we were executing
     */
    inst.word = *(unsigned long *)(regs->_pc - ARM_INST_SIZE);

    if ((inst.swi.rsv1 != SWI_RSV1_VALUE) || (inst.swi.swi_number != SYSCALL_SWI))
    {
        /*
         * Not a syscall.  Don't handle it
         */
        return 0;
    }

    func = regs->_r0;
    arg1 = regs->_r1;
    arg2 = regs->_r2;
    arg3 = regs->_r3;
    arg4 = *((unsigned int *)(regs->_sp + 4));

#if DEBUG_SYSCALL
    bsp_printf("handle_syscall: func<%d> args<0x%x, 0x%x, 0x%x, 0x%x>...\n",
               func, arg1, arg2, arg3, arg4);
    bsp_printf("Register Dump (0x%08lx):\n", (unsigned long)regs);
    _bsp_dump_regs(regs);
#endif

    if (func == SYS_exit) {
	regs->_pc = regs->_r12;

        /*
         *  We want to stop in exit so that the user may poke around
         *  to see why his app exited.
         */
        (*bsp_shared_data->__dbg_vector)(exc_nr, regs);

        return 1;
    }

    if (func == SYS_interrupt) {
	regs->_pc = regs->_r12;
        /*
         *  A console interrupt landed us here.
	 *  Invoke the debug agent so as to cause a SIGINT.
         */
        (*bsp_shared_data->__dbg_vector)(BSP_CORE_EXC_IRQ, regs);

        return 1;
    }

    if (_bsp_do_syscall(func, arg1, arg2, arg3, arg4, &err)) {
        regs->_r0 = err;
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

#if BSP_MAX_BP > 0
int
_bsp_set_breakpoint(bp_t *bp)
{
    int n;

    if (bp->flags & BP_IN_MEMORY)
	return 0;

    n = bsp_memory_read(bp->address, -1, 8, 4, (void *)(&bp->old_inst));
    if (n != 4)
	return 1;
    n = bsp_memory_write(bp->address, -1, 8, 4, (void *)bsp_breakinsn);
    if (n != 4)
	return 1;
    bp->flags |= BP_IN_MEMORY;

    bsp_flush_dcache(bp->address, 4);
    bsp_flush_icache(bp->address, 4);
    return 0;
}


int
_bsp_clear_breakpoint(bp_t *bp)
{
    int n;

    if (bp->flags & BP_IN_MEMORY) {
	n = bsp_memory_write(bp->address, -1, 8, 4, (void *)(&bp->old_inst));
	if (n != 4)
	    return 1;
	bp->flags &= ~BP_IN_MEMORY;
	bsp_flush_dcache(bp->address, 4);
	bsp_flush_icache(bp->address, 4);
    }
    return 0;
}
#endif


#ifdef SUPPORT_ADP
extern void _bsp_adp_handler(int exc_nr, void *regs);
#endif

/*
 *  Architecture specific BSP initialization.
 */
void
_bsp_cpu_init(void)
{
    static bsp_vec_t sc_vec;

#ifdef __CPU_SHARP_LH77790A__
    /*
     * Install cpu specific data
     */
    bsp_shared_data->__cpu_data = &cpu_data;
#endif /* __CPU_SHARP_LH77790A__ */

#ifdef __CPU_SA1100__
     _bsp_platform_info.cpu = "SA-1100 StrongARM(R) Microprocessor";
     _bsp_platform_info.extra = "StrongARM is a Registered Trademark of Advanced RISC Machines Limited.\nIntel is a Registered Trademark of Intel Corporation\nOther Brands and Trademarks are the property of their respective owners.";

    _bsp_dcache_info.size = SA1100_DCACHE_SIZE;
    _bsp_dcache_info.linesize = SA1100_DCACHE_LINESIZE_BYTES;
    _bsp_dcache_info.ways = SA1100_DCACHE_WAYS;

    _bsp_icache_info.size = SA1100_ICACHE_SIZE;
    _bsp_icache_info.linesize = SA1100_ICACHE_LINESIZE_BYTES;
    _bsp_icache_info.ways = SA1100_ICACHE_WAYS;

    bsp_shared_data->__flush_icache = __icache_flush;
    bsp_shared_data->__flush_dcache = __dcache_flush;

    bsp_shared_data->__reset = __board_reset;
#endif

#ifdef __CPU_SA110__
    _bsp_platform_info.cpu = "SA-110 StrongARM(R) Microprocessor";
    _bsp_platform_info.extra = "StrongARM is a Registered Trademark of Advanced RISC Machines Limited.\nIntel is a Registered Trademark of Intel Corporation\nOther Brands and Trademarks are the property of their respective owners.";

    _bsp_dcache_info.size = SA110_DCACHE_SIZE;
    _bsp_dcache_info.linesize = SA110_DCACHE_LINESIZE_BYTES;
    _bsp_dcache_info.ways = SA110_DCACHE_WAYS;

    _bsp_icache_info.size = SA110_ICACHE_SIZE;
    _bsp_icache_info.linesize = SA110_ICACHE_LINESIZE_BYTES;
    _bsp_icache_info.ways = SA110_ICACHE_WAYS;

    bsp_shared_data->__flush_icache = __icache_flush;
    bsp_shared_data->__flush_dcache = __dcache_flush;
#endif

#ifdef __XSCALE__
    _bsp_platform_info.cpu = "XScale(R) Microprocessor";
    _bsp_platform_info.extra = "XScale is a Registered Trademark of Intel Corporation\nOther Brands and Trademarks are the property of their respective owners.";

    /* FIXME! - cache info.  */
    _bsp_dcache_info.size = 32 * 1024;
    _bsp_dcache_info.linesize = 32;
    _bsp_dcache_info.ways = 32;

    _bsp_icache_info.size = 32 * 1024;
    _bsp_icache_info.linesize = 32;
    _bsp_icache_info.ways = 32;

    bsp_shared_data->__flush_icache = __icache_flush;
    bsp_shared_data->__flush_dcache = __dcache_flush;

#endif

    /*
     * Install syscall handler.
     */
    sc_vec.handler = syscall_handler;
    sc_vec.next    = NULL;
    bsp_install_vec(BSP_VEC_EXCEPTION, BSP_CORE_EXC_SOFTWARE_INTERRUPT,
                    BSP_VEC_REPLACE, &sc_vec);

#if 0
#ifdef SUPPORT_ADP
    bsp_install_dbg_handler(_bsp_adp_handler);
#endif
#endif
}
