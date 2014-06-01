/*
 * arm710t.h -- CPU specific header for Cygnus BSP.
 *   ARM(R) 710T
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
 *
 * Intel is a Registered Trademark of Intel Corporation.
 * Other Brands and Trademarks are the property of their respective owners.
 */

#ifndef __ARM710T_H__
#define __ARM710T_H__ 1

#include <bsp/defs.h>

/*
 * Make sure the MMU is compiled in
 */
#define MMU

#ifdef __ASSEMBLER__

#define REG8_VAL(a)  (a)
#define REG16_VAL(a) (a)
#define REG32_VAL(a) (a)

#define REG8_PTR(a)  (a)
#define REG16_PTR(a) (a)
#define REG32_PTR(a) (a)

#else /* __ASSEMBLER__ */

#define REG8_VAL(a)  ((unsigned char)(a))
#define REG16_VAL(a) ((unsigned short)(a))
#define REG32_VAL(a) ((unsigned int)(a))

#define REG8_PTR(a)  ((volatile unsigned char *)(a))
#define REG16_PTR(a) ((volatile unsigned long *)(a))
#define REG32_PTR(a) ((volatile unsigned long *)(a))

#endif /* __ASSEMBLER__ */

/*
 * 710T Cache and MMU Control Registers
 *
 * Accessed through coprocessor instructions.
 */
#define ARM710T_ID_REGISTER                        0
#define ARM710T_CONTROL_REGISTER                   1
#define ARM710T_TRANSLATION_TABLE_BASE_REGISTER    2
#define ARM710T_DOMAIN_ACCESS_CONTROL_REGISTER     3
#define ARM710T_FAULT_STATUS_REGISTER              5
#define ARM710T_FAULT_ADDRESS_REGISTER             6
#define ARM710T_CACHE_OPERATIONS_REGISTER          7
#define ARM710T_TLB_OPERATIONS_REGISTER            8

/*
 * 710T Cache and MMU Definitions
 */
#define ARM710T_ICACHE_SIZE                       SZ_8K
#define ARM710T_DCACHE_SIZE                       SZ_8K
#define ARM710T_ICACHE_LINESIZE_BYTES             16
#define ARM710T_DCACHE_LINESIZE_BYTES             16
#define ARM710T_ICACHE_LINESIZE_WORDS             4
#define ARM710T_DCACHE_LINESIZE_WORDS             4
#define ARM710T_ICACHE_WAYS                       4
#define ARM710T_DCACHE_WAYS                       4
#define ARM710T_ICACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(ARM710T_ICACHE_LINESIZE_BYTES - 1))
#define ARM710T_DCACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(ARM710T_DCACHE_LINESIZE_BYTES - 1))

#define ARM710T_ZEROS_BANK_BASE                  (0x50000000)

/*
 * 710T Cache and MMU ID Register value
 */
#define ARM710T_ID_MASK                            0xFFFFFFF0
#define ARM710T_ID_VALUE                           0x41807100

/*
 * 710T Cache Control Register Bit Fields and Masks
 */
#define ARM710T_MMU_DISABLED                       0x00000000
#define ARM710T_MMU_ENABLED                        0x00000001
#define ARM710T_MMU_MASK                           0x00000001
#define ARM710T_ADDRESS_FAULT_DISABLED             0x00000000
#define ARM710T_ADDRESS_FAULT_ENABLED              0x00000002
#define ARM710T_ADDRESS_FAULT_MASK                 0x00000002
#define ARM710T_CACHE_DISABLED                     0x00000000
#define ARM710T_CACHE_ENABLED                      0x00000004
#define ARM710T_CACHE_MASK                         0x00000004
#define ARM710T_WRITE_BUFFER_DISABLED              0x00000000
#define ARM710T_WRITE_BUFFER_ENABLED               0x00000008
#define ARM710T_WRITE_BUFFER_MASK                  0x00000008
#define ARM710T_LITTLE_ENDIAN                      0x00000000
#define ARM710T_BIG_ENDIAN                         0x00000080
#define ARM710T_ACCESS_CHECKS_SYSTEM               0x00000100
#define ARM710T_ACCESS_CHECKS_ROM                  0x00000200

/*
 * 710T Translation Table Base Bit Masks
 */
#define ARM710T_TRANSLATION_TABLE_MASK             0xFFFFC000

/*
 * 710T Domain Access Control Bit Masks
 */
#define ARM710T_DOMAIN_0_MASK                      0x00000003
#define ARM710T_DOMAIN_1_MASK                      0x0000000C
#define ARM710T_DOMAIN_2_MASK                      0x00000030
#define ARM710T_DOMAIN_3_MASK                      0x000000C0
#define ARM710T_DOMAIN_4_MASK                      0x00000300
#define ARM710T_DOMAIN_5_MASK                      0x00000C00
#define ARM710T_DOMAIN_6_MASK                      0x00003000
#define ARM710T_DOMAIN_7_MASK                      0x0000C000
#define ARM710T_DOMAIN_8_MASK                      0x00030000
#define ARM710T_DOMAIN_9_MASK                      0x000C0000
#define ARM710T_DOMAIN_10_MASK                     0x00300000
#define ARM710T_DOMAIN_11_MASK                     0x00C00000
#define ARM710T_DOMAIN_12_MASK                     0x03000000
#define ARM710T_DOMAIN_13_MASK                     0x0C000000
#define ARM710T_DOMAIN_14_MASK                     0x30000000
#define ARM710T_DOMAIN_15_MASK                     0xC0000000

/*
 * 710T Fault Status Bit Masks
 */
#define ARM710T_FAULT_STATUS_MASK                  0x0000000F
#define ARM710T_DOMAIN_MASK                        0x000000F0

/*
 * 710T Cache Control Operations Definitions
 */
#define ARM710T_FLUSH_CACHE_INST_DATA_OPCODE       0x0
#define ARM710T_FLUSH_CACHE_INST_DATA_RM           0x7

/*
 * 710T TLB Operations Definitions
 */
#define ARM710T_FLUSH_INST_DATA_TLB_OPCODE         0x0
#define ARM710T_FLUSH_INST_DATA_TLB_RM             0x7
#define ARM710T_FLUSH_DATA_ENTRY_TLB_OPCODE        0x1

#endif /* __ARM710T_H__ */
