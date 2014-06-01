/* Definitions of target machine for Mitsubishi D10V.
   Copyright (C) 1999, 2000, 2001, 2002, 2003, 2009, 2010
   Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

/* Variables to hold compare operators until the appropriate branch or scc
   operation is done.  */
extern GTY(()) rtx d10v_compare_op0;
extern GTY(()) rtx d10v_compare_op1;

/* Functions defined in d10v.c.  */
extern void debug_stack_info           (d10v_stack_t *);
extern int  direct_return              (void);
extern int  parallel_ldi               (HOST_WIDE_INT);
extern d10v_stack_t * d10v_stack_info  (void);

#ifdef RTX_CODE
extern int  gpr_operand (rtx, enum machine_mode);
extern int  accum_operand (rtx, enum machine_mode);
extern int  reg_or_0_operand (rtx, enum machine_mode);
extern int  reg_or_short_memory_operand (rtx, enum machine_mode);
extern int  arith16_operand (rtx, enum machine_mode);
extern int  arith_4bit_operand (rtx, enum machine_mode);
extern int  arith_nonnegative_operand (rtx, enum machine_mode);
extern int  arith32_operand (rtx, enum machine_mode);
extern int  arith64_operand (rtx, enum machine_mode);
extern int  arith_lower0_operand (rtx, enum machine_mode);
extern int  ldi_shift_operand (rtx, enum machine_mode);
extern int  pc_or_label_operand (rtx, enum machine_mode);
extern int  short_memory_operand (rtx, enum machine_mode);
extern int  cond_move_operand (rtx, enum machine_mode);
extern int  cond_exec_operand (rtx, enum machine_mode);
extern int  carry_operand (rtx, enum machine_mode);
extern int  f0_operand (rtx, enum machine_mode);
extern int  f1_operand (rtx, enum machine_mode);
extern int  f_operand (rtx, enum machine_mode);
extern int  f0_compare_operator (rtx, enum machine_mode);
extern int  compare_operator (rtx, enum machine_mode);
extern int  equality_operator (rtx, enum machine_mode);
extern int  signed_compare_operator (rtx, enum machine_mode);
extern int  unsigned_compare_operator (rtx, enum machine_mode);
extern int  unary_parallel_operator (rtx, enum machine_mode);
extern int  binary_parallel_operator (rtx, enum machine_mode);
extern int  extend_parallel_operator (rtx, enum machine_mode);
extern int  minmax_parallel_operator (rtx, enum machine_mode);
extern int  adjacent_memory_operands (rtx, rtx, rtx, int);
extern void d10v_expand_divmod (rtx *, int);
extern enum machine_mode d10v_emit_comparison  (enum rtx_code, rtx, rtx, rtx, rtx, enum machine_mode, int);
extern void d10v_expand_branch (enum rtx_code, rtx);
extern void d10v_expand_setcc (enum rtx_code, rtx);
extern rtx  d10v_expand_compare (enum rtx_code);
extern int  d10v_expand_movcc (rtx[]);
extern const char *emit_move_word (rtx, rtx, rtx);
extern const char *emit_move_2words (rtx, rtx, rtx);
extern const char *emit_add (rtx *, rtx);
extern const char *emit_subtract (rtx *, rtx);
extern const char *emit_cond_move (rtx *, rtx);
extern rtx  d10v_split_logical_op (rtx *, enum rtx_code);
extern void d10v_output_addr_const (FILE *, rtx);
extern void print_operand_address (FILE *, rtx);
extern void print_operand (FILE *, rtx, int);
extern rtx  d10v_subword (rtx, int, enum machine_mode, enum machine_mode);
extern rtx  d10v_expand_builtin_va_arg (tree, tree);
extern void d10v_expand_prologue (void);
extern void d10v_expand_epilogue (void);

#ifdef TREE_CODE
extern void init_cumulative_args (CUMULATIVE_ARGS *, tree, rtx, tree, int);
extern rtx  function_arg         (CUMULATIVE_ARGS *, enum machine_mode, tree, int);
#endif
#endif

#ifdef TREE_CODE
extern int  function_arg_boundary       (enum machine_mode, tree);
extern int  function_arg_callee_copies  (CUMULATIVE_ARGS *, enum machine_mode, tree, int);
extern void function_arg_advance        (CUMULATIVE_ARGS *, enum machine_mode, tree, int);
#endif
