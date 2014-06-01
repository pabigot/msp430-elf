/*
 * xscale.h -- CPU specific header for Cygnus BSP.
 *   Intel(R) XScale
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
 * Other Brands and Trademarks are the property of their respective owners.
 */
#ifndef __XSCALE_H__
#define __XSCALE_H__ 1

/*
 * Make sure the MMU is compiled in
 */
#define MMU

/* Other CP15 registers when opcode_2 == 0 */
#ifdef __ASSEMBLER__
#  define ARM_CACHETYPE_REGISTER                 c0
#  define ARM_AUX_CONTROL_REGISTER               c1
#  define ARM_COP_ACCESS__REGISTER               c13
#else /* __ASSEMBLER__ */
#  define ARM_CACHETYPE_REGISTER                 "c0"
#  define ARM_AUX_CONTROL_REGISTER               "c1"
#  define ARM_COP_ACCESS__REGISTER               "c13"
#endif /* __ASSEMBLER__ */

/* CP15 registers when opcode_2 == 1 */
#ifdef __ASSEMBLER__
#  define ARM_CACHETYPE_REGISTER                 c0
#  define ARM_AUX_CONTROL_REGISTER               c1
#  define ARM_COP_ACCESS__REGISTER               c13
#else /* __ASSEMBLER__ */
#  define ARM_CACHETYPE_REGISTER                 "c0"
#  define ARM_AUX_CONTROL_REGISTER               "c1"
#  define ARM_COP_ACCESS__REGISTER               "c13"
#endif /* __ASSEMBLER__ */

/* CP13 provides the interrupt controller */
#ifdef __ASSEMBLER__
#  define XSCALE_INTERRUPT_CP                    p13
#  define XSCALE_INTERRUPT_INTCTL_REGISTER       cr0
#  define XSCALE_INTERRUPT_INTSRC_REGISTER       cr4
#  define XSCALE_INTERRUPT_INTSTR_REGISTER       cr8
#else /* __ASSEMBLER__ */
#  define XSCALE_INTERRUPT_CP                    "p13"
#  define XSCALE_INTERRUPT_INTCTL_REGISTER       "cr0"
#  define XSCALE_INTERRUPT_INTSRC_REGISTER       "cr4"
#  define XSCALE_INTERRUPT_INTSTR_REGISTER       "cr8"
#endif /* __ASSEMBLER__ */

/*
 * CP13 defines 4 interrupt source for the XScale architecture:
 */
#define XSCALE_NUM_INTERRUPTS                    4

#define XSCALE_INTSRC_INDEX_IRQ                  0
#define XSCALE_INTSRC_INDEX_FIQ                  1
#define XSCALE_INTSRC_INDEX_BCU                  2
#define XSCALE_INTSRC_INDEX_PMU                  3

#define XSCALE_INTSRC_INDEX_FIRST                XSCALE_INTSRC_INDEX_IRQ
#define XSCALE_INTSRC_INDEX_LAST                 XSCALE_INTSRC_INDEX_PMU

#define XSCALE_INTCTL_nFIQ_MASK                  0x1
#define XSCALE_INTCTL_nIRQ_MASK                  0x2
#define XSCALE_INTCTL_PMU_MASK                   0x4
#define XSCALE_INTCTL_BCU_MASK                   0x8

#define XSCALE_INTSRC_nFIQ_ACTIVE                0x80000000
#define XSCALE_INTSRC_nIRQ_ACTIVE                0x40000000
#define XSCALE_INTSRC_BCU_ACTIVE                 0x20000000
#define XSCALE_INTSRC_PMU_ACTIVE                 0x10000000

#define XSCALE_INTSTR_BCU_TO_IRQ                 0x0
#define XSCALE_INTSTR_BCU_TO_FIQ                 0x2
#define XSCALE_INTSTR_PMU_TO_IRQ                 0x0
#define XSCALE_INTSTR_PMU_TO_FIQ                 0x1

#ifdef __YAVAPAI__
#define YAVAPAI_PCI_INT_ROUTING                  ((volatile unsigned long *)0x1050)
#define YAVAPAI_IRQ0_TO_PCI_INT                  0x0
#define YAVAPAI_IRQ0_TO_FIQ                      0x1
#define YAVAPAI_IRQ1_TO_PCI_INT                  0x0
#define YAVAPAI_IRQ1_TO_FIQ                      0x2
#define YAVAPAI_IRQ2_TO_PCI_INT                  0x0
#define YAVAPAI_IRQ2_TO_FIQ                      0x4
#define YAVAPAI_IRQ3_TO_PCI_INT                  0x0
#define YAVAPAI_IRQ3_TO_FIQ                      0x8
#endif

#endif /* __XSCALE_H__ */
