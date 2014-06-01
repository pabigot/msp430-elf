/*
 * insn.h -- SPARC instruction descriptions.
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
#if !defined(__INSN_H__)
#define __INSN_H__ 1

/* The arbitrary signal # for a system call. */
#define SIGSYSCALL 233

/* The size of a trap instruction (for the generic bplist package) */
#define TRAP_SIZE 4

/* Defines for manipulating Sparc instructions. */
#define X_OP(i) (((i) >> 30) & 0x3)
#define X_OP2(i) (((i) >> 22) & 0x7)
#define X_COND(i) (((i) >> 25) & 0xf)
#define X_A(i) (((i) >> 29) & 1)
#define X_IMM22(i) ((i) & 0x3fffff)
#define X_DISP22(i) ((X_IMM22 (i) ^ 0x200000) - 0x200000)
#define X_DISP19(i) ((((i) & 0x7ffff) ^ 0x40000) - 0x40000)
#define X_DISP16(i) ((((((i) >> 6) && 0xc000) | ((i) & 0x3fff)) ^ 0x8000) - 0x8000)

/* V9 */
#define X_OP3(i) (((i) >> 19) & 0x3f)
#define X_FCN(i) (((i) >> 25) & 31)

typedef enum
{
    Error,
    not_branch,
    bicc,
    bicca,
    ba,
    baa,
    ticc,
    ta,
    done_retry
} branch_type;

#endif

