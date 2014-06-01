/*
 * gdb-cpu.c -- CPU specific support for GDB stub.
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
#include <string.h>
#include <bsp/cpu.h>
#include <bsp/bsp.h>
#include "insn.h"
#include "gdb.h"

/*
 * Return byte offset within the saved register area of the
 * given register.
 */
int
bsp_regbyte(int regno)
{
    long  fp_end;
    
    if (regno < REG_F0)
	return regno * sizeof(long);


    if (regno < (REG_F0 + 32))
	return (long)&((ex_regs_t *)0L)->_fpr[regno - REG_F0];

    fp_end = (long)&((ex_regs_t *)0L)->_fpr[32];
    return fp_end + ((regno - (REG_F0 + 32)) * sizeof(long));
}


/*
 * Return size in bytes of given register.
 */
int
bsp_regsize(int regno)
{
    if (regno >= REG_F0 && regno <= REG_F31) {
#ifdef __R3000
	return sizeof(float);
#else
	return sizeof(double);
#endif
    }

    return sizeof(long);
}


#ifndef __R3000
void
bsp_set_register(int regno, void *saved_regs, void *val)
{
    unsigned long u;

    if (regno == REG_PC) {
	memcpy(&u, val, 8);
	bsp_set_pc(u, saved_regs);
    } else if (regno == REG_EPC) {
	memcpy(saved_regs + bsp_regbyte(regno), val, 8);
	if ((((ex_regs_t *)saved_regs)->_sr & SR_ERL) == 0)
	    memcpy(saved_regs + bsp_regbyte(REG_PC), val, 8);
    } else if (regno == REG_EEPC) {
	memcpy(saved_regs + bsp_regbyte(regno), val, 8);
	if (((ex_regs_t *)saved_regs)->_sr & SR_ERL)
	    memcpy(saved_regs + bsp_regbyte(REG_PC), val, 8);
    } else
	memcpy(saved_regs + bsp_regbyte(regno), val, 8);
}


void
bsp_get_register(int regno, void *saved_regs, void *val)
{
    memcpy(val, (char *)saved_regs + bsp_regbyte(regno), bsp_regsize(regno));
}
#endif

/*
 * Asynchronous interrupt support.
 * This is a hack for eCos to set a breakpoint instruction from
 * within its serial driver to fix asynchronous interruption
 * from gdb.
 */
static struct {
  unsigned *targetAddr;
  unsigned savedInstr;
} asyncBuffer;

/* Called to asynchronously interrupt a running program.
   Must be passed address of instruction interrupted.
   This is typically called in response to a debug port
   receive interrupt.
*/

void
__install_async_breakpoint(void *epc)
{
    long gp_save;

    /* This may be called from a separately linked program,
       so we need to save and restore the incoming gp register
       and setup our stub local gp register. 
       Alternatively, we could skip this foolishness if we
       compiled libstub with "-G 0". */

    __asm__ volatile ( "move   %0,$28\n"
		       ".extern _gp\n"
		       "la     $28,_gp\n"
		       : "=r" (gp_save) );

    asyncBuffer.targetAddr = epc;
    asyncBuffer.savedInstr = *(unsigned *)epc;
    *(unsigned *)epc = BREAKPOINT_OPCODE;
    bsp_flush_dcache(epc, 4);
    bsp_flush_icache(epc, 4);

    __asm__  volatile ( "move   $28,%0\n" :: "r"(gp_save) );
}


/*
 *  Given an exception number and a pointer to saved registers,
 *  return a GDB signal value.
 */
int
bsp_get_signal(int exc_nr, void *saved_regs)
{
    int sig = TARGET_SIGNAL_TRAP;

    if (asyncBuffer.targetAddr) {
	*asyncBuffer.targetAddr = asyncBuffer.savedInstr;
	bsp_flush_dcache(asyncBuffer.targetAddr, 4);
	bsp_flush_icache(asyncBuffer.targetAddr, 4);
	asyncBuffer.targetAddr = (unsigned *)0;
	return TARGET_SIGNAL_INT;
    }

    switch (exc_nr) {
      case BSP_EXC_TLB:
      case BSP_EXC_XTLB:
      case BSP_EXC_TLBMOD:
      case BSP_EXC_TLBL:
      case BSP_EXC_TLBS:
      case BSP_EXC_VCEI:
      case BSP_EXC_VCED:
	sig = TARGET_SIGNAL_SEGV;
	break;

      case BSP_EXC_ADEL:
      case BSP_EXC_ADES:
      case BSP_EXC_IBE:
      case BSP_EXC_DBE:
      case BSP_EXC_CACHE:
	sig = TARGET_SIGNAL_BUS;
	break;

      case BSP_EXC_ILL:
      case BSP_EXC_CPU:
	sig = TARGET_SIGNAL_ILL;
	break;

      case BSP_EXC_FPE:
      case BSP_EXC_OV:
	sig = TARGET_SIGNAL_FPE;
	break;

      case BSP_EXC_INT:
      case BSP_EXC_NMI:
	sig = TARGET_SIGNAL_INT;
	break;
    }

    return sig;
}


/*
 * Set the PC value in the saved registers.
 */
void
bsp_set_pc(unsigned long pc, void *saved_regs)
{
    ex_regs_t *regs = (ex_regs_t *)saved_regs;

    regs->_pc = pc;

#ifndef __R3000
    if (regs->_sr & SR_ERL)
	regs->_eepc = pc;
    else
	regs->_epc = pc;
#endif
}


/*
 * Get the PC value from the saved registers.
 */
unsigned long
bsp_get_pc(void *saved_regs)
{
    return ((ex_regs_t *)saved_regs)->_pc;
}


void
bsp_copy_gdb_to_exc_regs(ex_regs_t *ex_regs, gdb_regs_t *gdb_regs)
{
    /* for mips, our exception frame matches the gdb layout */
    memcpy(ex_regs, gdb_regs, sizeof(gdb_regs_t));
}

void
bsp_copy_exc_to_gdb_regs(gdb_regs_t *gdb_regs, ex_regs_t *ex_regs)
{
    /* for mips, our exception frame matches the gdb layout */
    memcpy(gdb_regs, ex_regs, sizeof(gdb_regs_t));
}


/*
 * Disable interrupts for saved context. Return true if 
 * interrupts were enabled. Return value has no meaning
 * for enable function.
 */
int
bsp_set_interrupt_enable(int disable, void *saved_regs)
{
    unsigned sr;
    static unsigned saved_ena;

    sr = __get_sr();

    if (disable) {
	saved_ena = sr & 1;
	sr &= ~1;
    } else {
	sr |= saved_ena;
    }

    __set_sr(sr);

    return 1;
}
