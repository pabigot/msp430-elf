/*
 * xscale.c -- Intel(R) XScale specific support for Cygnus BSP
 *
 * Copyright (c) 2000 Red Hat, Inc.
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
 * XScale is a Registered Trademark of Intel Corporation.
 * ARM is a Registered Trademark of Advanced RISC Machines Limited.
 * Other Brands and Trademarks are the property of their respective owners.
 */
#include <stdlib.h>
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/xscale.h>
#include "bsp_if.h"
#include "queue.h"

#define DEBUG_XSCALE_INT_CODE 0

static void	xscale_ictrl_init     (const struct bsp_irq_controller * ic);
static int	xscale_ictrl_disable  (const struct bsp_irq_controller * ic, int irq_nr);
static void	xscale_ictrl_enable   (const struct bsp_irq_controller * ic, int irq_nr);

static bsp_vec_t * xscale_irq_vec_tbl[XSCALE_NUM_INTERRUPTS];

static const struct bsp_irq_controller xscale_irq_controller =
{
    XSCALE_INTSRC_INDEX_FIRST, XSCALE_INTSRC_INDEX_LAST,
    &xscale_irq_vec_tbl[0],
    xscale_ictrl_init,
    xscale_ictrl_disable,
    xscale_ictrl_enable
};

/*
 * Dispatch code for xscale IRQ interrupts. Called by exception dispatch code.
 */
static int
xscale_irq_dispatch(int ex_nr, ex_regs_t *regs)
{
  bsp_vec_t * vec = xscale_irq_vec_tbl[XSCALE_INTSRC_INDEX_IRQ - XSCALE_INTSRC_INDEX_FIRST];
  int done = 0;
  
#if DEBUG_XSCALE_INT_CODE
  bsp_printf("xscale_irq_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
  BSP_ASSERT(ex_nr == BSP_CORE_EXC_IRQ);
#endif

  while (!done && vec && vec->handler != NULL)
    {
#if DEBUG_XSCALE_INT_CODE
      bsp_printf("Dispatching to 0x%08lx\n", vec->handler);
#endif
      done = vec->handler (ex_nr, regs);
      vec = vec->next;
    }
  
#if DEBUG_XSCALE_INT_CODE
      bsp_printf("Returning from xscale_irq_dispatch\n");
#endif
  return done;
}

/*
 * Dispatch code for xscale FIQ interrupts. Called by exception dispatch code.
 */
static int
xscale_fiq_dispatch(int ex_nr, ex_regs_t *regs)
{
  bsp_vec_t * vec = xscale_irq_vec_tbl[XSCALE_INTSRC_INDEX_FIQ - XSCALE_INTSRC_INDEX_FIRST];
  int done = 0;
  
#if DEBUG_XSCALE_INT_CODE
  bsp_printf("xscale_fiq_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
  BSP_ASSERT(ex_nr == BSP_CORE_EXC_FIQ);
#endif

  while (!done && vec && vec->handler != NULL)
    {
#if DEBUG_XSCALE_INT_CODE
      bsp_printf("Dispatching to 0x%08lx\n", vec->handler);
#endif
      done = vec->handler (ex_nr, regs);
      vec = vec->next;
    }
  
  return done;
}

/*
 * Dispatch code for xscale BCU interrupts. Called by exception dispatch code.
 */
static int
xscale_bcu_dispatch(int ex_nr, ex_regs_t *regs)
{
  bsp_vec_t * vec = xscale_irq_vec_tbl[XSCALE_INTSRC_INDEX_BCU - XSCALE_INTSRC_INDEX_FIRST];
  int done = 0;
  
#if DEBUG_XSCALE_INT_CODE
  bsp_printf("xscale_bcu_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
  BSP_ASSERT((ex_nr == BSP_CORE_EXC_IRQ) || (ex_nr == BSP_CORE_EXC_FIQ));
#endif

  while (!done && vec && vec->handler != NULL)
    {
#if DEBUG_XSCALE_INT_CODE
      bsp_printf("Dispatching to 0x%08lx\n", vec->handler);
#endif
      done = vec->handler (ex_nr, regs);
      vec = vec->next;
    }
  
  return done;
}

/*
 * Dispatch code for xscale PMU interrupts. Called by exception dispatch code.
 */
static int
xscale_pmu_dispatch(int ex_nr, ex_regs_t *regs)
{
  bsp_vec_t * vec = xscale_irq_vec_tbl[XSCALE_INTSRC_INDEX_PMU - XSCALE_INTSRC_INDEX_FIRST];
  int done = 0;
  
#if DEBUG_XSCALE_INT_CODE
  bsp_printf("xscale_pmu_dispatch(%d, 0x%08lx)\n", ex_nr, regs);
  BSP_ASSERT((ex_nr == BSP_CORE_EXC_IRQ) || (ex_nr == BSP_CORE_EXC_FIQ));
#endif

  while (!done && vec && vec->handler != NULL)
    {
#if DEBUG_XSCALE_INT_CODE
      bsp_printf("Dispatching to 0x%08lx\n", vec->handler);
#endif
      done = vec->handler (ex_nr, regs);
      vec = vec->next;
    }
  
  return done;
}

/*
 * Initialize XSCALE interrupt controller.
 */
static void
xscale_ictrl_init (const struct bsp_irq_controller * ic)
{
  static bsp_vec_t irq_vec, fiq_vec, bcu_vec1, bcu_vec2, pmu_vec1, pmu_vec2;
  int i;
  
  for (i = ic->first; i < ic->last; i++)
    ic->vec_list[i - ic->first] = NULL;

  /*
   * Setup exception handlers for the exceptions
   * corresponding to all interrupt sources
   */
  irq_vec.handler = (void *)xscale_irq_dispatch;
  irq_vec.next = NULL;
  bsp_install_vec (BSP_VEC_EXCEPTION, BSP_CORE_EXC_IRQ, BSP_VEC_CHAIN_FIRST, &irq_vec);
  
  fiq_vec.handler = (void *)xscale_fiq_dispatch;
  fiq_vec.next = NULL;
  bsp_install_vec (BSP_VEC_EXCEPTION, BSP_CORE_EXC_FIQ, BSP_VEC_CHAIN_FIRST, &fiq_vec);

  bcu_vec1.handler = (void *)xscale_bcu_dispatch;
  bcu_vec1.next = NULL;
  bsp_install_vec (BSP_VEC_EXCEPTION, BSP_CORE_EXC_IRQ, BSP_VEC_CHAIN_FIRST, &bcu_vec1);
  bcu_vec2.handler = (void *)xscale_bcu_dispatch;
  bcu_vec2.next = NULL;
  bsp_install_vec (BSP_VEC_EXCEPTION, BSP_CORE_EXC_FIQ, BSP_VEC_CHAIN_FIRST, &bcu_vec2);

  pmu_vec1.handler = (void *)xscale_pmu_dispatch;
  pmu_vec1.next = NULL;
  bsp_install_vec (BSP_VEC_EXCEPTION, BSP_CORE_EXC_IRQ, BSP_VEC_CHAIN_FIRST, &pmu_vec1);
  pmu_vec2.handler = (void *)xscale_pmu_dispatch;
  pmu_vec2.next = NULL;
  bsp_install_vec (BSP_VEC_EXCEPTION, BSP_CORE_EXC_FIQ, BSP_VEC_CHAIN_FIRST, &pmu_vec2);
}

/*
 * Disable XSCALE interrupts.
 */
static int
xscale_ictrl_disable (const struct bsp_irq_controller * ic, int irq_nr)
{
  unsigned long int_enable_mask = 0;
  register unsigned long intctl_value;

  /*
   * Determine the correct mask to use to __ENABLE__ this interrupt source.
   */
  switch (irq_nr)
    {
    case (XSCALE_INTSRC_INDEX_IRQ - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_nIRQ_MASK; break;
    case (XSCALE_INTSRC_INDEX_FIQ - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_nFIQ_MASK; break;
    case (XSCALE_INTSRC_INDEX_BCU - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_BCU_MASK; break;
    case (XSCALE_INTSRC_INDEX_PMU - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_PMU_MASK; break;
    }

  /*
   * Get the current intctl register value
   */
  __mrc(XSCALE_INTERRUPT_CP,
        ARM_COPROCESSOR_OPCODE_DONT_CARE,
        intctl_value,
        XSCALE_INTERRUPT_INTCTL_REGISTER,
        ARM_COPROCESSOR_RM_DONT_CARE,
        ARM_COPROCESSOR_OPCODE_DONT_CARE);

  /*
   * Clear the bit - ie disable this interrupt source
   */
  intctl_value &=~ int_enable_mask;

  /*
   * Now write the new intctl register value
   */
  __mcr(XSCALE_INTERRUPT_CP,
        ARM_COPROCESSOR_OPCODE_DONT_CARE,
        intctl_value,
        XSCALE_INTERRUPT_INTCTL_REGISTER,
        ARM_COPROCESSOR_RM_DONT_CARE,
        ARM_COPROCESSOR_OPCODE_DONT_CARE);

  return 0;
}

/*
 * Enable XSCALE interrupts.
 */
static void
xscale_ictrl_enable (const struct bsp_irq_controller * ic, int irq_nr)
{
  unsigned long int_enable_mask = 0;
  register unsigned long intctl_value;

  /*
   * Determine the correct mask to use to __ENABLE__ this interrupt source.
   */
  switch (irq_nr)
    {
    case (XSCALE_INTSRC_INDEX_IRQ - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_nIRQ_MASK; break;
    case (XSCALE_INTSRC_INDEX_FIQ - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_nFIQ_MASK; break;
    case (XSCALE_INTSRC_INDEX_BCU - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_BCU_MASK; break;
    case (XSCALE_INTSRC_INDEX_PMU - XSCALE_INTSRC_INDEX_FIRST): int_enable_mask = XSCALE_INTCTL_PMU_MASK; break;
    }

  /*
   * Get the current intctl register value
   */
  __mrc(XSCALE_INTERRUPT_CP,
        ARM_COPROCESSOR_OPCODE_DONT_CARE,
        intctl_value,
        XSCALE_INTERRUPT_INTCTL_REGISTER,
        ARM_COPROCESSOR_RM_DONT_CARE,
        ARM_COPROCESSOR_OPCODE_DONT_CARE);

  /*
   * Clear the bit - ie disable this interrupt source
   */
  intctl_value |= int_enable_mask;

  /*
   * Now write the new intctl register value
   */
  __mcr(XSCALE_INTERRUPT_CP,
        ARM_COPROCESSOR_OPCODE_DONT_CARE,
        intctl_value,
        XSCALE_INTERRUPT_INTCTL_REGISTER,
        ARM_COPROCESSOR_RM_DONT_CARE,
        ARM_COPROCESSOR_OPCODE_DONT_CARE);

  return;
}

/* XSCALE specific routine to 'install' interrupt controllers.  */

void
_bsp_install_xscale_irq_controllers (void)
{
#if DEBUG_XSCALE_INT_CODE
    bsp_printf("_bsp_install_xscale_irq_controllers()\n");
#endif /* DEBUG_XSCALE_INT_CODE */7

  _bsp_install_irq_controller(&xscale_irq_controller);

#if DEBUG_XSCALE_INT_CODE
    bsp_printf("Done w/ _bsp_install_xscale_irq_controllers()\n");
#endif /* DEBUG_XSCALE_INT_CODE */
}

/* Flush entire Icache.  */

void
__icache_flush (void *__p, int __nbytes)
{
  __mcr (ARM_CACHE_COPROCESSOR_NUM,
	 ARM_COPROCESSOR_OPCODE_DONT_CARE,
	 0,
	 ARM_CACHE_OPERATIONS_REGISTER,
	 ARM_FLUSH_CACHE_INST_RM,
	 ARM_FLUSH_CACHE_INST_OPCODE);
  
  __cpwait ();
}

/* Flush entire Dcache.  */
#define DCACHE_LSIZE 32

void
__dcache_flush (void * __p, int __nbytes)
{
  if (__nbytes > 0)
    {
      unsigned lp = (unsigned)__p;

      __nbytes += (lp & (DCACHE_LSIZE - 1));
      lp &= ~(DCACHE_LSIZE - 1);
      
      __nbytes +=  (DCACHE_LSIZE - 1);
      __nbytes &= ~(DCACHE_LSIZE - 1);
      
      while (__nbytes > 0)
	{
	  asm volatile ("mcr p15,0,%0,c7,c10,1" : /* no outputs */ : "r" (lp));

	  __nbytes -= DCACHE_LSIZE;
	  lp += DCACHE_LSIZE;
	}
      
      /* Now drain the write buffer.  */
      asm volatile ("mcr p15,0,r0,c7,c10,4");
    }
  else if (__nbytes < 0)
    {
      int i;
      unsigned lp = 0xa1000000;
      
      for (i = 0; i < 1024; i++)
	{
	  asm volatile ("mcr p15,0,%0,c7,c2,5" : /* no outputs */ : "r" (lp));
	  lp += DCACHE_LSIZE;
	}
      
      /* Now drain the write buffer.  */
      asm volatile ("mcr p15,0,r0,c7,c10,4");
      
      /* Now invalidate the cache.  */
      asm volatile ("mcr p15,0,r0,c7,c6,0");
    }
      
  __cpwait ();
}

