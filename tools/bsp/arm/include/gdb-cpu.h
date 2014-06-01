/*
 * gdb-cpu.h -- CPU specific definitions for GDB stub.
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
#ifndef __GDB_CPU_H__
#define __GDB_CPU_H__ 1

/*
 * Number of registers that gdb is interested in.
 */
#define NUMREGS 26

#define IS_GDB_T_REG(r) (((r)==REG_PC)||((r)==REG_SP))

#endif /* __GDB_CPU_H__ */
