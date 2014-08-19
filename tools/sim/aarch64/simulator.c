/* simulator.c -- Interface for the AArch64 simulator.

   Copyright (C) 2013-2014 by Red Hat.

   This file is part of the GNU simulators.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
   
       (1) Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer. 
   
       (2) Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in
       the documentation and/or other materials provided with the
       distribution.  
       
       (3)The name of the author may not be used to
       endorse or promote products derived from this software without
       specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.  */ 

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <syscall.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "dis-asm.h"

#include "simulator.h"
#include "cpustate.h"
#include "memory.h"

#include "../../libgloss/aarch64/svc.h"

#define NO_SP 0
#define SP_OK 1

int trace = 0;
int disas = 0;

static u_int32_t  instr;
static ErrorCode  errorCode;
static StatusCode status;

#define TST( _flag )   (aarch64_test_CPSR_bit (_flag))
#define IS_SET( _X )   ( TST (( _X )))
#define IS_CLEAR( _X ) (!TST (( _X )))

static StatusCode dexLogicalShiftedRegister (void);
static StatusCode dexCondSelect (void);

#define return_UNALLOC							\
  do									\
    {									\
      /* if (trace & TRACE_MISC) */					\
      {									\
	aarch64_print_insn (aarch64_get_PC ());				\
	fprintf (stderr, "\nUnallocated instruction detected at sim line %d, exe addr %" PRINT_64 "x\n", \
		 __LINE__, aarch64_get_PC ());				\
      }									\
      errorCode = ERROR_UNALLOC;					\
      return STATUS_ERROR;						\
    }									\
  while (1)

#define return_NYI							\
  do									\
    {									\
      /* if (trace & TRACE_MISC) */					\
	{								\
	  aarch64_print_insn (aarch64_get_PC ());			\
	  fprintf (stderr, "\nUnimplemented instruction detected at sim line %d, exe addr %" PRINT_64 "x\n", \
		   __LINE__, aarch64_get_PC ());			\
	}								\
      errorCode = ERROR_NYI;						\
      return STATUS_ERROR;						\
    }									\
  while (1)

#define NYI_assert(HI, LO, EXPECTED)			\
  do							\
    {							\
      if (uimm (instr, (HI), (LO)) != (EXPECTED))	\
	return_NYI;					\
    }							\
  while (0)

StatusCode
aarch64_set_status (StatusCode state, ErrorCode err)
{
  errorCode = err;
  return status = state;
}

StatusCode
aarch64_get_status (void)
{
  return status;
}

ErrorCode
aarch64_get_ErrorCode (void)
{
  return errorCode;
}


/* Possible arguments to report_and_die().  */
#define UNIMPLEMENTED_INSTRUCTION 1
#define INVALID_OPERAND 2
#define INTERNAL_ERROR 3

int
report_and_die (int exitCode)
{
  if (trace & TRACE_MISC)
    fprintf (stderr, "Exiting simulator with exit code %d\n", exitCode);
  exit (exitCode);
  return STATUS_ERROR;
}


#define TOP_LEVEL_RETURN_PC 0xffffffffffffffecULL

#define SIM_STACK_SIZE (64 * 1024)

static u_int64_t * stack = NULL;  /* Full, descending.  */
static u_int64_t stack_limit = 0; /* Number of empty slots remaining.  */

u_int64_t *
aarch64_get_stack (void)
{
  if (stack == NULL)
    {
      size_t amount;

      stack_limit = SIM_STACK_SIZE;
      amount = stack_limit * sizeof (* stack);
      
      stack = malloc (amount);
      if (stack)
	stack += stack_limit + amount;
    }

  return stack;
}

u_int64_t *
aarch64_get_end_of_stack (void)
{
  u_int64_t * s = aarch64_get_stack ();

  if (s != NULL)
    return s - stack_limit;

  return NULL;
}

// helper functions used by expandLogicalImmediate

// for i = 1, ... N result<i-1> = 1 other bits are zero
static inline u_int64_t
ones (int N)
{
  return (N == 64 ? (u_int64_t)-1UL : ((1UL << N) - 1));
}

// result<0> to val<N>
static inline u_int64_t
pickbit (u_int64_t val, int N)
{
  return pickbits64(val, N, N);
}

static u_int64_t
expand_logical_immediate (u_int32_t S, u_int32_t R, u_int32_t N)
{
  u_int64_t mask;
  u_int64_t imm;
  unsigned simd_size;

  /* The immediate value is S+1 bits to 1, left rotated by SIMDsize - R
     (in other words, right rotated by R), then replicated. */
  if (N != 0)
    {
      simd_size = 64;
      mask = 0xffffffffffffffffull;
    }
  else
    {
      switch (S)
	{
	case 0x00 ... 0x1f: /* 0xxxxx */ simd_size = 32;           break;
	case 0x20 ... 0x2f: /* 10xxxx */ simd_size = 16; S &= 0xf; break;
	case 0x30 ... 0x37: /* 110xxx */ simd_size =  8; S &= 0x7; break;
	case 0x38 ... 0x3b: /* 1110xx */ simd_size =  4; S &= 0x3; break;
	case 0x3c ... 0x3d: /* 11110x */ simd_size =  2; S &= 0x1; break;
	default: return 0;
	}
      mask = (1ull << simd_size) - 1;
      /* Top bits are IGNORED.  */
      R &= simd_size - 1;
    }

  /* NOTE: if S = simd_size - 1 we get 0xf..f which is rejected.  */
  if (S == simd_size - 1)
    return 0;

  /* S+1 consecutive bits to 1.  */
  /* NOTE: S can't be 63 due to detection above.  */
  imm = (1ull << (S + 1)) - 1;

  /* Rotate to the left by simd_size - R.  */
  if (R != 0)
    imm = ((imm << (simd_size - R)) & mask) | (imm >> R);

  /* Replicate the value according to SIMD size.  */
  switch (simd_size)
    {
    case  2: imm = (imm <<  2) | imm;
    case  4: imm = (imm <<  4) | imm;
    case  8: imm = (imm <<  8) | imm;
    case 16: imm = (imm << 16) | imm;
    case 32: imm = (imm << 32) | imm;
    case 64: break;
    default: return 0;
    }

  return imm;
}

// instr[22,10] encodes N immr and imms. we want a lookup table
// for each possible combination i.e. 13 bits worth of int entries
#define  LI_TABLE_SIZE  (1 << 13)
static u_int64_t LITable[LI_TABLE_SIZE];

static void
init_LIT_table (void)
{
  unsigned index;

  for (index = 0; index < LI_TABLE_SIZE; index++)
    {
      u_int32_t N    = uimm (index, 12, 12);
      u_int32_t immr = uimm (index, 11, 6);
      u_int32_t imms = uimm (index, 5, 0);

      LITable [index] = expand_logical_immediate (imms, immr, N);
    }
}

static StatusCode
dexNotify (void)
{
  // instr[14,0] == type : 0 ==> method entry, 1 ==> method reentry
  //                       2 ==> exit Java, 3 ==> start next bytecode
  u_int32_t type = uimm (instr, 14, 0);

  if (trace & TRACE_MISC)
    fprintf (stderr, "Notify Insn encountered, type = 0x%x\n", type);

  switch (type)
    {
    case 0:
      // aarch64_notifyMethodEntry (aarch64_get_reg_u64 (R23, 0), aarch64_get_reg_u64 (R22, 0));
      break;
    case 1:
      // aarch64_notifyMethodReentry (aarch64_get_reg_u64 (R23, 0), aarch64_get_reg_u64 (R22, 0));
      break;
    case 2:
      // aarch64_notifyMethodExit ();
      break;
    case 3:
      // aarch64_notifyBCStart (aarch64_get_reg_u64 (R23, 0), aarch64_get_reg_u64 (R22, 0));
      break;
    }
  return status;
}

// secondary decode within top level groups

static StatusCode
dexPseudo (void)
{
  // assert instr[28,27] = 00
  //
  // we provide 2 pseudo instructions
  //
  //  HALT stops execution of the simulator causing an immediate
  //  return to the x86 code which entered it
  //
  //  CALLOUT initiates recursive entry into x86 code. a register
  //  argument holds the address of the x86 routine.  Immediate
  //  values in the instruction identify the number of general
  //  purpose and floating point register arguments to be passed
  //  and the type of any value to be returned
  //
  u_int32_t PSEUDO_HALT      =  0xE0000000U;
  u_int32_t PSEUDO_CALLOUT   =  0x00018000U;
  u_int32_t PSEUDO_CALLOUTR  =  0x00018001U;
  u_int32_t PSEUDO_NOTIFY    =  0x00014000U;

  if (instr == PSEUDO_HALT)
    {
      if (trace & TRACE_MISC)
	fprintf (stderr, " Pseudo Halt Instruction\n");
      return STATUS_HALT;
    }

  u_int32_t dispatch = uimm (instr, 31, 15);

  if (dispatch == PSEUDO_CALLOUT || dispatch == PSEUDO_CALLOUTR)
    // Punt this for handling in outer loop.
    return STATUS_CALLOUT;
  
  if (dispatch == PSEUDO_NOTIFY)
    return dexNotify ();

  return_UNALLOC;
}

// load-store single register (unscaled offset)
// these instructions employ a base register plus an unscaled signed
// 9 bit offset
//
// n.b. the base register (source) can be Xn or SP. all other
// registers may not be SP.

// 32 bit load 32 bit unscaled signed 9 bit
static StatusCode
ldur32 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u32
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 64 bit load 64 bit unscaled signed 9 bit
static StatusCode
ldur64 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u64
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 32 bit load zero-extended byte unscaled signed 9 bit
static StatusCode
ldurb32 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u8
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 32 bit load sign-extended byte unscaled signed 9 bit
static StatusCode
ldursb32 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rt, NO_SP, (u_int32_t) aarch64_get_mem_s8
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 64 bit load sign-extended byte unscaled signed 9 bit
static StatusCode
ldursb64 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  return aarch64_set_reg_s64 (rt, NO_SP, aarch64_get_mem_s8
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 32 bit load zero-extended short unscaled signed 9 bit
static StatusCode
ldurh32 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_mem_u16
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 32 bit load sign-extended short unscaled signed 9 bit
static StatusCode
ldursh32 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP, (u_int32_t) aarch64_get_mem_s16
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 64 bit load sign-extended short unscaled signed 9 bit
static StatusCode
ldursh64 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  return aarch64_set_reg_s64 (rt, NO_SP, aarch64_get_mem_s16
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 64 bit load sign-extended word unscaled signed 9 bit
static StatusCode
ldursw (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP, (u_int32_t) aarch64_get_mem_s32
			      (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// n.b. with stores the value in source is written to the address
// identified by source2 modified by offset

// 32 bit store 32 bit unscaled signed 9 bit
static StatusCode
stur32 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_mem_u32 (aarch64_get_reg_u64 (rn, SP_OK) + offset,
			      aarch64_get_reg_u32 (rd, NO_SP));
}

// 64 bit store 64 bit unscaled signed 9 bit
static StatusCode
stur64 (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_mem_u64 (aarch64_get_reg_u64 (rn, SP_OK) + offset,
			      aarch64_get_reg_u64 (rd, NO_SP));
}

// 32 bit store byte unscaled signed 9 bit
static StatusCode
sturb (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_mem_u8 (aarch64_get_reg_u64 (rn, SP_OK) + offset,
			     aarch64_get_reg_u8 (rd, NO_SP));
}

// 32 bit store short unscaled signed 9 bit
static StatusCode
sturh (int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_mem_u16 (aarch64_get_reg_u64 (rn, SP_OK) + offset,
			      aarch64_get_reg_u16 (rd, NO_SP));
}

// load single register pc-relative label
// offset is a signed 19 bit immediate count in words
// rt may not be SP
// 32 bit pc-relative load
static StatusCode
ldr32_pcrel (int32_t offset)
{
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_mem_u32 (aarch64_get_PC () + offset * 4));
}

// 64 bit pc-relative load
static StatusCode
ldr_pcrel (int32_t offset)
{
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_mem_u64 (aarch64_get_PC () + offset * 4));
}

// sign extended 32 bit pc-relative load
static StatusCode
ldrsw_pcrel (int32_t offset)
{
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_mem_s32 (aarch64_get_PC () + offset * 4));
}

// float pc-relative load
static StatusCode
fldrs_pcrel (int32_t offset)
{
  unsigned int rd = uimm (instr, 4, 0);

  return aarch64_set_FP_float (rd, aarch64_get_mem_float (aarch64_get_PC () + offset * 4));
}

// double pc-relative load
static StatusCode
fldrd_pcrel (int32_t offset)
{
  unsigned int st = uimm (instr, 4, 0);

  return aarch64_set_FP_double (st, aarch64_get_mem_double (aarch64_get_PC () + offset * 4));
}

// long double pc-relative load
static StatusCode
fldrq_pcrel (int32_t offset)
{
  unsigned int st = uimm (instr, 4, 0);
  u_int64_t addr = aarch64_get_PC () + offset * 4;
  FRegister a;

  aarch64_get_mem_long_double (addr, & a);
  return aarch64_set_FP_long_double (st, a);
}

// this can be used to scale an offset by applying
// the requisite shift. the second argument is either
// 16, 32 or 64

#define SCALE(_offset, _elementSize) \
    ((_offset) << ScaleShift ## _elementSize)

// this can be used to optionally scale a register derived offset
// by applying the requisite shift as indicated by the Scaling
// argument. the second argument is either Byte, Short, Word
// or Long. The third argument is either Scaled or Unscaled.
// n.b. when _Scaling is Scaled the shift gets ANDed with
// all 1s while when it is Unscaled it gets ANDed with 0

#define OPT_SCALE(_offset, _elementType, _Scaling) \
  ((_offset) << (_Scaling ? ScaleShift ## _elementType : 0))

// this can be used to zero or sign extend a 32 bit register derived
// value to a 64 bit value.  the first argument must be the value as
// a u_int32_t and the second must be either UXTW or SXTW. The result
// is returned as an int64_t

static inline int64_t
extend (u_int32_t value, Extension extension)
{
  // a branchless variant of this ought to be possible
  if (extension == UXTW || extension == NoExtension)
    return value;

  union
  {
    u_int32_t u;
    int32_t   n;
  } x;

  x.u = value;
  return x.n;
}

//// Scalar Floating Point
//

// FP load/store single register (4 addressing modes)
//
// n.b. the base register (source) can be the stack pointer.
// The secondary source register (source2) can only be an Xn register.

// load 32 bit unscaled signed 9 bit with pre- or post-writeback
static StatusCode
fldrs_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_FP_float (st, aarch64_get_mem_float (address));
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

//load 32 bit scaled unsigned 12 bit
static StatusCode
fldrs_abs (u_int32_t offset)
{
  unsigned st = uimm (instr, 4, 0);
  unsigned rn = uimm (instr, 9, 5);
  return aarch64_set_FP_float (st, aarch64_get_mem_float (aarch64_get_reg_u64 (rn, SP_OK)
							  + SCALE (offset, 32)));
}

// load 32 bit scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
fldrs_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement = OPT_SCALE (extended, 32, scaling);

  return aarch64_set_FP_float (st, aarch64_get_mem_float (address + displacement));
}

// load 64 bit unscaled signed 9 bit with pre- or post-writeback
static StatusCode
fldrd_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_FP_double (st, aarch64_get_mem_double (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

//load 64 bit scaled unsigned 12 bit
static StatusCode
fldrd_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 64);

  return aarch64_set_FP_double (st, aarch64_get_mem_double (address));
}

// load 64 bit scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
fldrd_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement = OPT_SCALE (extended, 64, scaling);

  return fldrd_wb (displacement, NoWriteBack);
}

// load 128 bit unscaled signed 9 bit with pre- or post-writeback
static StatusCode
fldrq_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  FRegister a;
  aarch64_get_mem_long_double (address, & a);
  aarch64_set_FP_long_double (st, a);

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// load 128 bit scaled unsigned 12 bit
static StatusCode
fldrq_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 128);

  FRegister a;
  aarch64_get_mem_long_double (address, & a);
  return aarch64_set_FP_long_double (st, a);
}

// load 128 bit scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
fldrq_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement = OPT_SCALE (extended, 128, scaling);

  return fldrq_wb (displacement, NoWriteBack);
}

//// Memory Access

// load-store single register
// there are four addressing modes available here which all employ a
// 64 bit source (base) register
//
// n.b. the base register (source) can be the stack pointer.
// The secondary source register (source2)can only be an Xn register.
//
// Scaled, 12-bit, unsigned immediate offset, without pre- and
// post-index options.
// Unscaled, 9-bit, signed immediate offset with pre- or post-index
// writeback.
// scaled or unscaled 64-bit register offset.
// scaled or unscaled 32-bit extended register offset.
//
// all offsets are assumed to be raw from the decode i.e. the
// simulator is expected to adjust scaled offsets based on the
// accessed data size with register or extended register offset
// versions the same applies except that in the latter case the
// operation may also require a sign extend
//
// a separate method is provided for each possible addressing mode

// 32 bit load 32 bit scaled unsigned 12 bit
static StatusCode
ldr32_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  // the target register may not be SP but the source may be
  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u32
			      (aarch64_get_reg_u64 (rn, SP_OK)
			       + SCALE (offset, 32)));
}

// 32 bit load 32 bit unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldr32_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u32 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit load 32 bit scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldr32_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement =  OPT_SCALE (extended, 32, scaling);

  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u32 (address + displacement));
}

// 64 bit load 64 bit scaled unsigned 12 bit
static StatusCode
ldr_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  // the target register may not be SP but the source may be
  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u64
			      (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 64)));
}

// 64 bit load 64 bit unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldr_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u64 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 64 bit load 64 bit scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldr_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement =  OPT_SCALE (extended, 64, scaling);

  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u64 (address + displacement));
}

// 32 bit load zero-extended byte scaled unsigned 12 bit
static StatusCode
ldrb32_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // the target register may not be SP but the source may be
  // there is no scaling required for a byte load
  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u8 (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// 32 bit load zero-extended byte unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldrb32_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u8 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit load zero-extended byte scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldrb32_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t displacement = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);

  // there is no scaling required for a byte load
  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u8 (address + displacement));
}

// 64 bit load sign-extended byte unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldrsb_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_s8 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 64 bit load sign-extended byte scaled unsigned 12 bit
static StatusCode
ldrsb_abs (u_int32_t offset)
{
  return ldrsb_wb (offset, NoWriteBack);
}

// 64 bit load sign-extended byte scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldrsb_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t displacement = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  // there is no scaling required for a byte load
  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_s8 (address + displacement));
}

// 32 bit load zero-extended short scaled unsigned 12 bit
static StatusCode
ldrh32_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // the target register may not be SP but the source may be
  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u16
			      (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 16)));
}

// 32 bit load zero-extended short unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldrh32_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u16 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit load zero-extended short scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldrh32_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement =  OPT_SCALE (extended, 16, scaling);

  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u16 (address + displacement));
}

// 32 bit load sign-extended short scaled unsigned 12 bit
static StatusCode
ldrsh32_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  // The target register may not be SP but the source may be.
  return aarch64_set_reg_u64 (rt, NO_SP, (u_int32_t) aarch64_get_mem_s16
			      (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 16)));
}

// 32 bit load sign-extended short unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldrsh32_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, (u_int32_t) aarch64_get_mem_s16 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit load sign-extended short scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldrsh32_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement =  OPT_SCALE (extended, 16, scaling);

  return aarch64_set_reg_u64 (rt, NO_SP, (u_int32_t) aarch64_get_mem_s16 (address + displacement));
}

// 64 bit load sign-extended short scaled unsigned 12 bit
static StatusCode
ldrsh_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  // The target register may not be SP but the source may be.
  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_s16
			      (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 16)));
}

// 64 bit load sign-extended short unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldrsh64_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_s16 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 64 bit load sign-extended short scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldrsh_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement = OPT_SCALE (extended, 16, scaling);

  return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_s16 (address + displacement));
}

// 64 bit load sign-extended 32 bit scaled unsigned 12 bit
static StatusCode
ldrsw_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  // The target register may not be SP but the source may be.
  return aarch64_set_reg_s64 (rt, NO_SP, aarch64_get_mem_s32
			      (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 32)));
}

// 64 bit load sign-extended 32 bit unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldrsw_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_reg_s64 (rt, NO_SP, aarch64_get_mem_s32 (address));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 64 bit load sign-extended 32 bit scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldrsw_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement =  OPT_SCALE (extended, 32, scaling);

  return aarch64_set_reg_s64 (rt, NO_SP, aarch64_get_mem_s32 (address + displacement));
}

// n.b. with stores the value in source is written to the address
// identified by source2 modified by source3/offset

// 32 bit store scaled unsigned 12 bit
static StatusCode
str32_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  // the target register may not be SP but the source may be
  return aarch64_set_mem_u32 ((aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 32)),
			      aarch64_get_reg_u32 (rt, NO_SP));
}

// 32 bit store unscaled signed 9 bit with pre- or post-writeback
static StatusCode
str32_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  if (wb != Post)
    address += offset;

  aarch64_set_mem_u32 (address, aarch64_get_reg_u32 (rt, NO_SP));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit store scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
str32_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t   extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement = OPT_SCALE (extended, 32, scaling);

  return aarch64_set_mem_u32 (address + displacement, aarch64_get_reg_u64 (rt, NO_SP));
}

// 64 bit store scaled unsigned 12 bit
static StatusCode
str_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  return aarch64_set_mem_u64 (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 64),
			      aarch64_get_reg_u64 (rt, NO_SP));
}

// 64 bit store unscaled signed 9 bit with pre- or post-writeback
static StatusCode
str_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_mem_u64 (address, aarch64_get_reg_u64 (rt, NO_SP));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 64 bit store scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
str_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t   extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement = OPT_SCALE (extended, 64, scaling);

  return aarch64_set_mem_u64 (address + displacement, aarch64_get_reg_u64 (rt, NO_SP));
}

// 32 bit store byte scaled unsigned 12 bit
static StatusCode
strb_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // the target register may not be SP but the source may be
  // there is no scaling required for a byte load
  return aarch64_set_mem_u8 (aarch64_get_reg_u64 (rn, SP_OK) + offset, aarch64_get_reg_u8 (rt, NO_SP));
}

// 32 bit store byte unscaled signed 9 bit with pre- or post-writeback
static StatusCode
strb_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_mem_u8 (address, aarch64_get_reg_u8 (rt, NO_SP));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit store byte scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
strb_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t displacement = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);

  // there is no scaling required for a byte load
  return aarch64_set_mem_u8 (address + displacement, aarch64_get_reg_u8 (rt, NO_SP));
}

// 32 bit store short scaled unsigned 12 bit
static StatusCode
strh_abs (u_int32_t offset)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  // the target register may not be SP but the source may be
  return aarch64_set_mem_u16 (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 16),
			      aarch64_get_reg_u16 (rt, NO_SP));
}

// 32 bit store short unscaled signed 9 bit with pre- or post-writeback
static StatusCode
strh_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);

  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;
  
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_mem_u16 (address, aarch64_get_reg_u16 (rt, NO_SP));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit store short scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
strh_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t displacement =  OPT_SCALE (extended, 16, scaling);

  return aarch64_set_mem_u16 (address + displacement, aarch64_get_reg_u16 (rt, NO_SP));
}

// prefetch unsigned 12 bit
static StatusCode
prfm_abs (u_int32_t offset)
{
  // instr[4,0] = prfop : 00000 ==> PLDL1KEEP, 00001 ==> PLDL1STRM,
  //                      00010 ==> PLDL2KEEP, 00001 ==> PLDL2STRM,
  //                      00100 ==> PLDL3KEEP, 00101 ==> PLDL3STRM,
  //                      10000 ==> PSTL1KEEP, 10001 ==> PSTL1STRM,
  //                      10010 ==> PSTL2KEEP, 10001 ==> PSTL2STRM,
  //                      10100 ==> PSTL3KEEP, 10101 ==> PSTL3STRM,
  //                      ow ==> UNALLOC
  // PrfOp prfop = prfop (instr, 4, 0);
  // u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 64);
  // TODO : implement prefetch of address
  return status;
}

// prefetch scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
prfm_scale_ext (Scaling scaling, Extension extension)
{
  // instr[4,0] = prfop : 00000 ==> PLDL1KEEP, 00001 ==> PLDL1STRM,
  //                      00010 ==> PLDL2KEEP, 00001 ==> PLDL2STRM,
  //                      00100 ==> PLDL3KEEP, 00101 ==> PLDL3STRM,
  //                      10000 ==> PSTL1KEEP, 10001 ==> PSTL1STRM,
  //                      10010 ==> PSTL2KEEP, 10001 ==> PSTL2STRM,
  //                      10100 ==> PSTL3KEEP, 10101 ==> PSTL3STRM,
  //                      ow ==> UNALLOC
  // rn may reference SP, rm may only reference ZR
  // PrfOp prfop = prfop(instr, 4, 0);
  // u_int64_t base = aarch64_get_reg_u64 (rn, SP_OK);
  // int64_t extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  // u_int64_t displacement =  OPT_SCALE (extended, 64, scaling);
  // u_int64_t address = base + displacement;
  // TODO : implement prefetch of address
  return status;
}

// 64 bit pc-relative prefetch
static StatusCode
prfm_pcrel (int32_t offset)
{
  // instr[4,0] = prfop : 00000 ==> PLDL1KEEP, 00001 ==> PLDL1STRM,
  //                      00010 ==> PLDL2KEEP, 00001 ==> PLDL2STRM,
  //                      00100 ==> PLDL3KEEP, 00101 ==> PLDL3STRM,
  //                      10000 ==> PSTL1KEEP, 10001 ==> PSTL1STRM,
  //                      10010 ==> PSTL2KEEP, 10001 ==> PSTL2STRM,
  //                      10100 ==> PSTL3KEEP, 10101 ==> PSTL3STRM,
  //                      ow ==> UNALLOC
  // PrfOp prfop = prfop (instr, 4, 0);
  // u_int64_t address = aarch64_get_PC () + offset
  // TODO : implement this
  return status;
}


// load-store pair
// IGNORE?

// load-store non-temporal pair
// IGNORE

// load-store unprivileged
// IGNORE?

// load-store exclusive

static StatusCode
ldxr (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int size = uimm (instr, 31, 30);
  // int ordered = uimm (instr, 15, 15);
  // int exclusive = ! uimm (instr, 23, 23);

  switch (size)
    {
    case 0: return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u8 (address));
    case 1: return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u16 (address));
    case 2: return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u32 (address));
    case 3: return aarch64_set_reg_u64 (rt, NO_SP, aarch64_get_mem_u64 (address));
    default: return_UNALLOC;
    }
}

static StatusCode
stxr (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rt = uimm (instr, 4, 0);
  unsigned rs = uimm (instr, 20, 16);
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int size = uimm (instr, 31, 30);
  // int ordered = uimm (instr, 15, 15);
  // int exclusive = ! uimm (instr, 23, 23);
  u_int64_t data = aarch64_get_reg_u64 (rt, NO_SP);
  StatusCode st;

  switch (size)
    {
    case 0: st = aarch64_set_mem_u8 (address, data); break;
    case 1: st = aarch64_set_mem_u16 (address, data); break;
    case 2: st = aarch64_set_mem_u32 (address, data); break;
    case 3: st = aarch64_set_mem_u64 (address, data); break;
    default: return_UNALLOC;
    }

  if (st == STATUS_READY)
    st = aarch64_set_reg_u64 (rs, NO_SP, 0); /* Always exclusive...  */
  return st;
}

static StatusCode
dexLoadLiteral (void)
{
  // assert instr[29,27] == 011 && instr[25,24] == 00
  // instr[31,30:26] = opc: 000 ==> LDRW,  001 ==> FLDRS
  //                        010 ==> LDRX,  011 ==> FLDRD
  //                        100 ==> LDRSW, 101 ==> FLDRQ
  //                        110 ==> PRFM, 111 ==> UNALLOC
  // instr[26] ==> V : 0 ==> GReg, 1 ==> FReg
  // instr[23, 5] == simm19
  // unsigned rt = uimm (instr, 4, 0);
  u_int32_t dispatch = ( (uimm (instr, 31, 30) << 1)
			| uimm (instr, 26, 26));
  int32_t imm = simm32 (instr, 23, 5);

  switch (dispatch)
    {
    case 0: return ldr32_pcrel (imm);
    case 1: return fldrs_pcrel (imm);
    case 2: return ldr_pcrel   (imm);
    case 3: return fldrd_pcrel (imm);
    case 4: return ldrsw_pcrel (imm);
    case 5: return fldrq_pcrel (imm);
    case 6: return prfm_pcrel  (imm);
    case 7:
    default:
      return_UNALLOC;
    }
}

// immediate arithmetic
// the aimm argument is a 12 bit unsigned value or a 12 bit unsigned
// value left shifted by 12 bits (done at decode)
//
// n.b. the register args (dest, source) can normally be Xn or SP.
// the exception occurs for flag setting instructions which may
// only use Xn for the output (dest)


// 32 bit add immediate
static StatusCode
add32 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u32 (rn, SP_OK) + aimm);
}

// 64 bit add immediate
static StatusCode
add64 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u64 (rn, SP_OK) + aimm);
}

static StatusCode
set_flags_for_add32 (int32_t value1, int32_t value2)
{
  int32_t   result = value1 + value2;
  int64_t   sresult = (int64_t) value1   + (int64_t) value2;
  u_int64_t uresult = (u_int64_t)(u_int32_t) value1 + (u_int64_t)(u_int32_t) value2;
  u_int32_t flags = 0;

  if (result == 0)
    flags |= Z;

  if (result & (1 << 31))
    flags |= N;

  if (uresult != result)
    flags |= C;

  if (sresult != result)
    flags |= V;

  return aarch64_set_CPSR (flags);
}

static StatusCode
set_flags_for_add64 (u_int64_t value1, u_int64_t value2)
{
  int64_t   sval1 = value1;
  int64_t   sval2 = value2;
  u_int64_t result = value1 + value2;
  int64_t   sresult = sval1 + sval2;
  u_int32_t flags = 0;

  if (result == 0)
    flags |= Z;

  if (result & (1ULL << 63))
    flags |= N;

  if (sval1 < 0)
    {
      if (sval2 < 0)
	{
	  /* Negative plus a negative.  Overflow happens if
	     the result is greater than either of the operands.  */
	  if (sresult > sval1 || sresult > sval2)
	    flags |= V;
	}
      else
	/* Negative plus a positive.  Overflow cannot happen.  */
	;
    }
  else /* value1 is +ve.  */
    {
      if (sval2 < 0)
	{
	  /* Overflow can only occur if we computed "0 - MININT".  */
	  if (sval1 == 0 && sval2 == (1LL << 63))
	    flags |= V;
	}
      else
	{
	  /* Postive plus positive - overflow has happened if the
	     result is smaller than either of the operands.  */
	  if (result < value1 || result < value2)
	    flags |= V | C;
	}
    }
  
  return aarch64_set_CPSR (flags);
}

#define NEG(a) (((a) & signbit) == signbit)
#define POS(a) (((a) & signbit) == 0)

static StatusCode
set_flags_for_sub32 (u_int32_t value1, u_int32_t value2)
{
  u_int32_t result = value1 - value2;
  u_int32_t flags = 0;
  u_int32_t signbit = 1ULL << 31;

  if (result == 0)
    flags |= Z;

  if (NEG (result))
    flags |= N;

  if (   (NEG (value1) && POS (value2))
      || (NEG (value1) && POS (result))
      || (POS (value2) && POS (result)))
    flags |= C;

  if (   (NEG (value1) && POS (value2) && POS (result))
      || (POS (value1) && NEG (value2) && NEG (result)))
    flags |= V;

  return aarch64_set_CPSR (flags);
}

static StatusCode
set_flags_for_sub64 (u_int64_t value1, u_int64_t value2)
{
  u_int64_t result = value1 - value2;
  u_int32_t flags = 0;
  u_int64_t signbit = 1ULL << 63;

  if (result == 0)
    flags |= Z;

  if (NEG (result))
    flags |= N;

  if (   (NEG (value1) && POS (value2))
      || (NEG (value1) && POS (result))
      || (POS (value2) && POS (result)))
    flags |= C;

  if (   (NEG (value1) && POS (value2) && POS (result))
      || (POS (value1) && NEG (value2) && NEG (result)))
    flags |= V;

  return aarch64_set_CPSR (flags);
}

static StatusCode
set_flags_for_binop32 (u_int32_t result)
{
  u_int32_t flags = 0;

  if (result == 0)
    flags |= Z;
  else
    flags &= ~ Z;

  if (result & (1 << 31))
    flags |= N;
  else
    flags &= ~ N;

  return aarch64_set_CPSR (flags);
}

static StatusCode
set_flags_for_binop64 (u_int64_t result)
{
  u_int32_t flags = 0;

  if (result == 0)
    flags |= Z;
  else
    flags &= ~ Z;

  if (result & (1ULL << 63))
    flags |= N;
  else
    flags &= ~ N;

  return aarch64_set_CPSR (flags);
}

// 32 bit add immediate set flags
static StatusCode
adds32 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  // TODO : do we need to worry about signs here?
  int32_t value1 = aarch64_get_reg_s32 (rn, SP_OK);

  aarch64_set_reg_u64 (rd, NO_SP, value1 + aimm);
  return set_flags_for_add32 (value1, aimm);
}

// 64 bit add immediate set flags
static StatusCode
adds64 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int64_t value1 = aarch64_get_reg_u64 (rn, SP_OK);
  u_int64_t value2 = aimm;

  aarch64_set_reg_u64 (rd, NO_SP, value1 + value2);
  return set_flags_for_add64 (value1, value2);
}

// 32 bit sub immediate
static StatusCode
sub32 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u32 (rn, SP_OK) - aimm);
}

// 64 bit sub immediate
static StatusCode
sub64 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u64 (rn, SP_OK) - aimm);
}

// 32 bit sub immediate set flags
static StatusCode
subs32 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int32_t value1 = aarch64_get_reg_u64 (rn, SP_OK);
  u_int32_t value2 = aimm;

  aarch64_set_reg_u64 (rd, NO_SP, value1 - value2);
  return set_flags_for_sub32 (value1, value2);
}

// 64 bit sub immediate set flags
static StatusCode
subs64 (u_int32_t aimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int64_t value1 = aarch64_get_reg_u64 (rn, SP_OK);
  u_int32_t value2 = aimm;

  aarch64_set_reg_u64 (rd, NO_SP, value1 - value2);
  return set_flags_for_sub64 (value1, value2);
}

// extract (immediate)
// IGNORE? only needed for ROR

//// Data Processing Register

// first two helpers to perform the shift operations

static inline u_int32_t
shifted32 (u_int32_t value, Shift shift, u_int32_t count)
{
  switch (shift)
    {
    default:
    case LSL:
      return (value << count);
    case LSR:
      return (value >> count);
    case ASR:
      {
	int32_t svalue = value;
	return (svalue >> count);
      }
    case ROR:
      {
	u_int32_t top = value >> count;
	u_int32_t bottom = value << (32 - count);
	return (bottom | top);
      }
    }
}

// first two helpers to perform the shift operations

static inline u_int64_t
shifted64 (u_int64_t value, Shift shift, u_int32_t count)
{
  switch (shift)
    {
    default:
    case LSL:
      return (value << count);
    case LSR:
      return (value >> count);
    case ASR:
      {
	int64_t svalue = value;
	return (svalue >> count);
      }
    case ROR:
      {
	u_int64_t top = value >> count;
	u_int64_t bottom = value << (64 - count);
	return (bottom | top);
      }
    }
}

// arithmetic shifted register
// these allow an optional LSL, ASR or LSR to the second source
// register with a count up to the register bit count.
//
// n.b register args may not be SP.

// 32 bit ADD shifted register
static StatusCode
add32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      + shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit ADD shifted register
static StatusCode
add64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      + shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

// 32 bit ADD shifted register setting flags
static StatusCode
adds32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t value2 = shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 + value2);
  return set_flags_for_add32 (value1, value2);
}

// 64 bit ADD shifted register setting flags
static StatusCode
adds64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t value2 = shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 + value2);
  return set_flags_for_add64 (value1, value2);
}

// 32 bit SUB shifted register
static StatusCode
sub32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      - shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit SUB shifted register
static StatusCode
sub64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      - shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

// 32 bit SUB shifted register setting flags
static StatusCode
subs32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t value2 = shifted32(aarch64_get_reg_u32 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 - value2);
  return set_flags_for_sub32 (value1, value2);
}

// 64 bit SUB shifted register setting flags
static StatusCode
subs64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t value2 = shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 - value2);
  return set_flags_for_sub64 (value1, value2);
}

// first a couple more helpers to fetch the
// relevant source register element either
// sign or zero extended as required by the
// extension value

static u_int32_t
extreg32 (unsigned int lo, Extension extension)
{
  switch (extension)
    {
    case UXTB:      return aarch64_get_reg_u8 (lo, NO_SP);
    case UXTH:      return aarch64_get_reg_u16 (lo, NO_SP);
    case UXTW: //deliberate drop-through
    case UXTX:      return aarch64_get_reg_u32 (lo, NO_SP);
    case SXTB:      return aarch64_get_reg_s8 (lo, NO_SP);
    case SXTH:      return aarch64_get_reg_s16 (lo, NO_SP);
    case SXTW: //deliberate drop-through
    case SXTX: //deliberate drop-through
    default:        return aarch64_get_reg_s32 (lo, NO_SP);
  }
}

static u_int64_t
extreg64 (unsigned int lo, Extension extension)
{
  switch (extension)
    {
    case UXTB:      return aarch64_get_reg_u8 (lo, NO_SP);
    case UXTH:      return aarch64_get_reg_u16 (lo, NO_SP);
    case UXTW:      return aarch64_get_reg_u32 (lo, NO_SP);
    case UXTX:      return aarch64_get_reg_u64 (lo, NO_SP);
    case SXTB:      return aarch64_get_reg_s8 (lo, NO_SP);
    case SXTH:      return aarch64_get_reg_s16 (lo, NO_SP);
    case SXTW:      return aarch64_get_reg_s32 (lo, NO_SP);
    case SXTX:
    default:        return aarch64_get_reg_s64 (lo, NO_SP);
    }
}

// arithmetic extending register
// these allow an optional sign extension of some portion of the
// second source register followed by an optional left shift of
// between 1 and 4 bits (i.e. a shift of 0-4 bits???)
//
// n.b output (dest) and first input arg (source) may normally be Xn
// or SP. However, for flag setting operations dest can only be
// Xn. Second input registers are always Xn

// 32 bit ADD extending register
static StatusCode
add32_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, SP_OK,
			      aarch64_get_reg_u32 (rn, SP_OK)
			      + (extreg32 (rm, extension) << shift));
}

// 64 bit ADD extending register
// n.b. this subsumes the case with 64 bit source2 and UXTX #n or LSL #0
static StatusCode
add64_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, SP_OK,
			      aarch64_get_reg_u64 (rn, SP_OK)
			      + (extreg64 (rm, extension) << shift));
}

// 32 bit ADD extending register setting flags
static StatusCode
adds32_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, SP_OK);
  u_int32_t value2 = extreg32 (rm, extension) << shift;

  aarch64_set_reg_u64 (rd, NO_SP, value1 + value2);
  return set_flags_for_add32 (value1, value2);
}

// 64 bit ADD extending register setting flags
// n.b. this subsumes the case with 64 bit source2 and UXTX #n or LSL #0
static StatusCode
adds64_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, SP_OK);
  u_int64_t value2 = extreg64 (rm, extension) << shift;

  aarch64_set_reg_u64 (rd, NO_SP, value1 + value2);
  return set_flags_for_add64 (value1, value2);
}

// 32 bit SUB extending register
static StatusCode
sub32_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u32 (rn, SP_OK)
			      - (extreg32 (rm, extension) << shift));
}

// 64 bit SUB extending register
// n.b. this subsumes the case with 64 bit source2 and UXTX #n or LSL #0
static StatusCode
sub64_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, SP_OK,  aarch64_get_reg_u64 (rn, SP_OK)
			      - (extreg64 (rm, extension) << shift));
}

// 32 bit SUB extending register setting flags
static StatusCode
subs32_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, SP_OK);
  u_int32_t value2 = extreg32 (rm, extension) << shift;

  aarch64_set_reg_u64 (rd, NO_SP, value1 - value2);
  return set_flags_for_sub32 (value1, value2);
}

// 64 bit SUB extending register setting flags
// n.b. this subsumes the case with 64 bit source2 and UXTX #n or LSL #0
static StatusCode
subs64_ext (Extension extension, u_int32_t shift)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, SP_OK);
  u_int64_t value2 = extreg64 (rm, extension) << shift;

  aarch64_set_reg_u64 (rd, NO_SP, value1 - value2);
  return set_flags_for_sub64 (value1, value2);
}

static StatusCode
dexAddSubtractImmediate (void)
{
  // instr[31]    = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30]    = op : 0 ==> ADD, 1 ==> SUB
  // instr[29]    = set : 0 ==> no flags, 1 ==> set flags
  // instr[28,24] = 10001
  // instr[23,22] = shift : 00 == LSL#0, 01 = LSL#12 1x = UNALLOC
  // instr[21,10] = uimm12
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd

  NYI_assert (28, 24, 0x11);
  
  // n.b. the shift is applied at decode before calling the add/sub routine
  u_int32_t shift = uimm (instr, 23, 22);

  if (shift > 1)
    return_UNALLOC;

  u_int32_t imm = uimm (instr, 21, 10);
  if (shift)
    imm <<= 12;

  u_int32_t dispatch = uimm (instr, 31, 29);

  switch (dispatch)
    {
    case 0: return add32 (imm);
    case 1: return adds32 (imm);
    case 2: return sub32 (imm);
    case 3: return subs32 (imm);
    case 4: return add64 (imm);
    case 5: return adds64 (imm);
    case 6: return sub64 (imm);
    case 7: return subs64 (imm);
    }
  return_UNALLOC;
}

static StatusCode
dexAddSubtractShiftedRegister (void)
{
  // instr[31]    = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30,29] = op : 00 ==> ADD, 01 ==> ADDS, 10 ==> SUB, 11 ==> SUBS
  // instr[28,24] = 01011 
  // instr[23,22] = shift : 0 ==> LSL, 1 ==> LSR, 2 ==> ASR, 3 ==> UNALLOC
  // instr[21]    = 0
  // instr[20,16] = Rm
  // instr[15,10] = count : must be 0xxxxx for 32 bit
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd

  NYI_assert (28, 24, 0x0B);
  NYI_assert (21, 21, 0);

  u_int32_t size = uimm (instr, 31, 31);

  // 32 bit operations must have count[5] = 0
  // or else we have an UNALLOC
  u_int32_t count = uimm (instr, 15, 10);

  if (!size && uimm (count, 5, 5))
    return_UNALLOC;

  // shift encoded as ROR is unallocated
  Shift shiftType = shift (instr, 22);
  if (shiftType == ROR)
    return_UNALLOC;

  // dispatch on size:op i.e instr[31,29]
  u_int32_t dispatch = uimm (instr, 31, 29);

  switch (dispatch)
    {
    case 0: return add32_shift (shiftType, count);
    case 1: return adds32_shift (shiftType, count);
    case 2: return sub32_shift (shiftType, count);
    case 3: return subs32_shift (shiftType, count);
    case 4: return add64_shift (shiftType, count);
    case 5: return adds64_shift (shiftType, count);
    case 6: return sub64_shift (shiftType, count);
    case 7: return subs64_shift (shiftType, count);
    default:
      return_UNALLOC;
    }
}

static StatusCode
dexAddSubtractExtendedRegister (void)
{
  // instr[31]    = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30]    = op : 0 ==> ADD, 1 ==> SUB
  // instr[29]    = set? : 0 ==> no flags, 1 ==> set flags
  // instr[28,24] = 01011
  // instr[23,22] = opt : 0 ==> ok, 1,2,3 ==> UNALLOC
  // instr[21]    = 1
  // instr[20,16] = Rm
  // instr[15,13] = option : 000 ==> UXTB, 001 ==> UXTH,
  //                         000 ==> LSL|UXTW, 001 ==> UXTZ,
  //                         000 ==> SXTB, 001 ==> SXTH,
  //                         000 ==> SXTW, 001 ==> SXTX,
  // instr[12,10] = shift : 0,1,2,3,4 ==> ok, 5,6,7 ==> UNALLOC
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd

  NYI_assert (28, 24, 0x0B);
  NYI_assert (21, 21, 1);
  
  Extension extensionType = extension (instr, 13);

  // shift may not exceed 4
  u_int32_t shift = uimm (instr, 12, 10);
  if (shift > 4)
    return_UNALLOC;

  // dispatch on size:op:set? i.e instr[31,29]
  u_int32_t dispatch = uimm (instr, 31, 29);

  switch (dispatch)
    {
    case 0: return add32_ext (extensionType, shift);
    case 1: return adds32_ext (extensionType, shift);
    case 2: return sub32_ext (extensionType, shift);
    case 3: return subs32_ext (extensionType, shift);
    case 4: return add64_ext (extensionType, shift);
    case 5: return adds64_ext (extensionType, shift);
    case 6: return sub64_ext (extensionType, shift);
    case 7: return subs64_ext (extensionType, shift);
    default: return_UNALLOC;
    }
}

// conditional data processing
// condition register is implicit 3rd source

// 32 bit add with carry
//
// n.b register args may not be SP.

static StatusCode
adc32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      + aarch64_get_reg_u32 (rm, NO_SP)
			      + IS_SET (C));
}

// 64 bit add with carry
static StatusCode
adc64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      + aarch64_get_reg_u64 (rm, NO_SP)
			      + IS_SET (C));
}

// 32 bit add with carry setting flags
static StatusCode
adcs32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t value2 = aarch64_get_reg_u32 (rm, NO_SP);
  u_int32_t carry = IS_SET (C);

  aarch64_set_reg_u64 (rd, NO_SP, value1 + value2 + carry);
  return set_flags_for_add32 (value1, value2 + carry);
}

// 64 bit add with carry setting flags
static StatusCode
adcs64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t value2 = aarch64_get_reg_u64 (rm, NO_SP);
  u_int64_t carry = IS_SET (C);

  aarch64_set_reg_u64 (rd, NO_SP, value1 + value2 + carry);
  return set_flags_for_add64 (value1, value2 + carry);
}

// 32 bit sub with carry
static StatusCode
sbc32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      - aarch64_get_reg_u32 (rm, NO_SP)
			      - 1 + IS_SET (C));
}

// 64 bit sub with carry
static StatusCode
sbc64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      - aarch64_get_reg_u64 (rm, NO_SP)
			      - 1 + IS_SET (C));
}

// 32 bit sub with carry setting flags
static StatusCode
sbcs32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t value2 = aarch64_get_reg_u32 (rm, NO_SP);
  u_int32_t carry  = IS_SET (C);
  u_int32_t result = value1 - value2 + 1 - carry;
  
  aarch64_set_reg_u64 (rd, NO_SP, result);
  return set_flags_for_sub32 (value1, value2 + 1 - carry);
}

// 64 bit sub with carry setting flags
static StatusCode
sbcs64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t value2 = aarch64_get_reg_u64 (rm, NO_SP);
  u_int64_t carry  = IS_SET (C);
  u_int64_t result = value1 - value2 + 1 - carry;

  aarch64_set_reg_u64 (rd, NO_SP, result);
  return set_flags_for_sub64 (value1, value2 + 1 - carry);
}

static StatusCode
dexAddSubtractWithCarry (void)
{
  // instr[31]    = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30]    = op : 0 ==> ADC, 1 ==> SBC
  // instr[29]    = set? : 0 ==> no flags, 1 ==> set flags
  // instr[28,21] = 11010000
  // instr[20,16] = Rm
  // instr[15,10] = op2 : 00000 ==> ok, ow ==> UNALLOC
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd

  NYI_assert (28, 21, 0xB0);

  u_int32_t op2 = uimm (instr, 15, 10);
  if (op2 != 0)
    return_UNALLOC;

  // dispatch on size:op:set? i.e instr[31,29]
  u_int32_t dispatch = uimm (instr, 31, 29);

  switch (dispatch)
    {
    case 0: return adc32 ();
    case 1: return adcs32 ();
    case 2: return sbc32 ();
    case 3: return sbcs32 ();
    case 4: return adc64 ();
    case 5: return adcs64 ();
    case 6: return sbc64 ();
    case 7: return sbcs64 ();
    default:
      return_UNALLOC;
    }
}

static u_int32_t
testConditionCode (CondCode cc)
{
  // this should be reduceable to branchless logic
  // by some careful testing of bits in CC followed
  // by the requisite masking and combining of bits
  // from the flag register

  // for now we do it with a switch

  int res;

  switch (cc)
    {
    case EQ:  res = IS_SET (Z);    break;
    case NE:  res = IS_CLEAR (Z);  break;
    case CS:  res = IS_SET (C);    break;
    case CC:  res = IS_CLEAR (C);  break;
    case MI:  res = IS_SET (N);    break;
    case PL:  res = IS_CLEAR (N);  break;
    case VS:  res = IS_SET (V);    break;
    case VC:  res = IS_CLEAR (V);  break;
    case HI:  res = IS_SET (C) && IS_CLEAR (Z);  break;
    case LS:  res = IS_CLEAR (C) || IS_SET (Z);  break;
    case GE:  res = IS_SET (N) == IS_SET (V);    break;
    case LT:  res = IS_SET (N) != IS_SET (V);    break;
    case GT:  res = IS_CLEAR (Z) && (IS_SET (N) == IS_SET (V));  break;
    case LE:  res = IS_SET (Z) || (IS_SET (N) != IS_SET (V));    break;
    case AL: 
    case NV:
    default:
      res = 1;
      break;
    }
  return res;
}

static StatusCode
CondCompare (void) // aka: ccmp
{
  // instr[31]    = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30,21] = 11 1101 0010
  // instr[20,16] = Rm or const
  // instr[15,12] = cond
  // instr[11]    = compare reg (0) or const (1)
  // instr[10]    = 0
  // instr[9,5]   = Rn
  // instr[4]     = 0
  // instr[3,0]   = flags

  NYI_assert (30, 21, 0x3d2);
  NYI_assert (10, 10, 0);
  NYI_assert (4, 4, 0);

  if (testConditionCode (uimm (instr, 15, 12)))
    {
      unsigned rm = uimm (instr, 20, 16);
      unsigned rn = uimm (instr,  9,  5);

      if (uimm (instr, 31, 31))
	{
	  if (uimm (instr, 11, 11))
	    return set_flags_for_add64 (aarch64_get_reg_u64 (rn, SP_OK), - (u_int64_t) rm);
	  else
	    return set_flags_for_add64 (aarch64_get_reg_u64 (rn, SP_OK),
					- aarch64_get_reg_u64 (rm, SP_OK));
	    
	}
      else
	{
	  if (uimm (instr, 11, 11))
	    return set_flags_for_add32 (aarch64_get_reg_u32 (rn, SP_OK), - rm);
	  else
	    return set_flags_for_add32 (aarch64_get_reg_u32 (rn, SP_OK),
					- aarch64_get_reg_u32 (rm, SP_OK));
	}
    }

  return aarch64_set_CPSR (uimm (instr, 3, 0));
}

static StatusCode
do_vec_MOV_whole_vector (void)
{
  // MOV Vd.T, Vs.T  (alias for ORR Vd.T, Vn.T, Vm.T where Vn == Vm)
  //
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,21] = 001110101
  // instr[20,16] = Vs
  // instr[15,10] = 000111
  // instr[9,5]   = Vs
  // instr[4,0]   = Vd

  NYI_assert (29, 21, 0x075);
  NYI_assert (15, 10, 0x07);

  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);

  if (uimm (instr, 20, 16) != vs)
    return_NYI;

  if (uimm (instr, 30, 30))
    aarch64_set_vec_u64 (vd, 1, aarch64_get_vec_u64 (vs, 1));

  return aarch64_set_vec_u64 (vd, 0, aarch64_get_vec_u64 (vs, 0));
}

static StatusCode
do_vec_MOV_into_scalar (void)
{
  // instr[31]    = 0
  // instr[30]    = word(0)/long(1)
  // instr[29,21] = 00 1110 000
  // instr[20,18] = element size and index
  // instr[17,10] = 00 0011 11
  // instr[9,5]   = V source
  // instr[4,0]   = R dest

  NYI_assert (29, 21, 0x070);
  NYI_assert (17, 10, 0x0F);

  unsigned vs = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  switch (uimm (instr, 20, 18))
    {
    case 0x2: return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u64 (vs, 0));
    case 0x6: return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u64 (vs, 1));

    case 0x1:
    case 0x3:
    case 0x5:
    case 0x7:
      return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u32 (vs, uimm (instr, 20, 19)));

    default:
      return_NYI;
    }
}

static StatusCode
do_vec_INS (void)
{
  // instr[31,21] = 01001110000
  // instr[20,16] = element size and index
  // instr[15,10] = 000111
  // instr[9,5]   = W source
  // instr[4,0]   = V dest

  NYI_assert (31, 21, 0x270);
  NYI_assert (15, 10, 0x07);
  
  int index;
  unsigned rs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);

  if (uimm (instr, 16, 16))
    {
      index = uimm (instr, 20, 17);
      return aarch64_set_vec_u8 (vd, index, aarch64_get_reg_u8 (rs, NO_SP));
    }

  if (uimm (instr, 17, 17))
    {
      index = uimm (instr, 20, 18);
      return aarch64_set_vec_u16 (vd, index, aarch64_get_reg_u16 (rs, NO_SP));
    }

  if (uimm (instr, 18, 18))
    {
      index = uimm (instr, 20, 19);
      return aarch64_set_vec_u32 (vd, index, aarch64_get_reg_u32 (rs, NO_SP));
    }

  if (uimm (instr, 19, 19))
    {
      index = uimm (instr, 20, 20);
      return aarch64_set_vec_u64 (vd, index, aarch64_get_reg_u64 (rs, NO_SP));
    }
  
  return_NYI;
}

static StatusCode
do_vec_DUP_vector_into_vector (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,21] = 00 1110 000
  // instr[20,16] = element size and index
  // instr[15,10] = 0000 01
  // instr[9,5]   = V source
  // instr[4,0]   = V dest

  NYI_assert (29, 21, 0x070);
  NYI_assert (15, 10, 0x01);

  unsigned full = uimm (instr, 30, 30);
  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  int i, index;

  if (uimm (instr, 16, 16))
    {
      index = uimm (instr, 20, 17);

      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_u8 (vd, i, aarch64_get_vec_u8 (vs, index));
    }
  else if (uimm (instr, 17, 17))
    {
      index = uimm (instr, 20, 18);

      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u16 (vs, index));
    }
  else if (uimm (instr, 18, 18))
    {
      index = uimm (instr, 20, 19);

      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u32 (vs, index));
    }
  else
    {
      if (uimm (instr, 19, 19) == 0)
	return_UNALLOC;

      if (! full)
	return_UNALLOC;

      index = uimm (instr, 20, 20);

      for (i = 0; i < 2; i++)
	aarch64_set_vec_u64 (vd, i, aarch64_get_vec_u64 (vs, index));
    }
  
  return status;
}

static StatusCode
do_vec_TBL (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,21] = 00 1110 000
  // instr[20,16] = Vm
  // instr[15]    = 0
  // instr[14,13] = vec length
  // instr[12,10] = 000
  // instr[9,5]   = V start
  // instr[4,0]   = V dest

  NYI_assert (29, 21, 0x070);
  NYI_assert (12, 10, 0);
  
  int full    = uimm (instr, 30, 30);
  int len     = uimm (instr, 14, 13) + 1;
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  
  for (i = 0; i < (full ? 16 : 8); i++)
    {
      unsigned int selector = aarch64_get_vec_u8 (vm, i);
      u_int8_t val;

      if (selector < 16)
	val = aarch64_get_vec_u8 (vn, selector);
      else if (selector < 32)
	val = len < 2 ? 0 : aarch64_get_vec_u8 (vn + 1, selector - 16);
      else if (selector < 48)
	val = len < 3 ? 0 : aarch64_get_vec_u8 (vn + 2, selector - 32);
      else if (selector < 64)
	val = len < 4 ? 0 : aarch64_get_vec_u8 (vn + 3, selector - 48);
      else
	val = 0;
      
      aarch64_set_vec_u8 (vd, i, val);
    }

  return status;
}

static StatusCode
do_vec_TRN (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,24] = 00 1110
  // instr[23,22] = size
  // instr[21]    = 0
  // instr[20,16] = Vm
  // instr[15]    = 0
  // instr[14]    = TRN1 (0) / TRN2 (1)
  // instr[13,10] = 1010
  // instr[9,5]   = V source
  // instr[4,0]   = V dest

  NYI_assert (29, 24, 0x0E);
  NYI_assert (13, 10, 0xA);

  int full    = uimm (instr, 30, 30);
  int second  = uimm (instr, 14, 14);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 8 : 4); i++)
	{
	  aarch64_set_vec_u8 (vd, i * 2, aarch64_get_vec_u8 (second ? vm : vn, i * 2));
	  aarch64_set_vec_u8 (vd, 1 * 2 + 1, aarch64_get_vec_u8 (second ? vn : vm, i * 2 + 1));
	}
      return status;

    case 1:
      for (i = 0; i < (full ? 4 : 2); i++)
	{
	  aarch64_set_vec_u16 (vd, i * 2, aarch64_get_vec_u16 (second ? vm : vn, i * 2));
	  aarch64_set_vec_u16 (vd, 1 * 2 + 1, aarch64_get_vec_u16 (second ? vn : vm, i * 2 + 1));
	}
      return status;

    case 2:
      aarch64_set_vec_u32 (vd, 0, aarch64_get_vec_u32 (second ? vm : vn, 0));
      aarch64_set_vec_u32 (vd, 1, aarch64_get_vec_u32 (second ? vn : vm, 1));
      aarch64_set_vec_u32 (vd, 2, aarch64_get_vec_u32 (second ? vm : vn, 2));
      return aarch64_set_vec_u32 (vd, 3, aarch64_get_vec_u32 (second ? vn : vm, 3));

    case 3:
      if (! full)
	return_UNALLOC;

      aarch64_set_vec_u64 (vd, 0, aarch64_get_vec_u64 (second ? vm : vn, 0));
      return aarch64_set_vec_u64 (vd, 1, aarch64_get_vec_u64 (second ? vn : vm, 1));

    default:
      return_UNALLOC;
    }
}

static StatusCode
do_vec_DUP_scalar_into_vector (void)
{
  // instr[31]    = 0
  // instr[30]    = 0=> zero top 64-bits, 1=> duplicate into top 64-bits [must be 1 for 64-bit xfer]
  // instr[29,20] = 00 1110 0000
  // instr[19,16] = element size: 0001=> 8-bits, 0010=> 16-bits, 0100=> 32-bits. 1000=>64-bits
  // instr[15,10] = 0000 11
  // instr[9,5]   = W source
  // instr[4,0]   = V dest

  NYI_assert (29, 20, 0x0E0);
  NYI_assert (15, 10, 0x03);

  unsigned i;
  unsigned Vd = uimm (instr, 4, 0);
  unsigned Rs = uimm (instr, 9, 5);
  int both    = uimm (instr, 30, 30);

  switch (uimm (instr, 19, 16))
    {
    case 1:
      for (i = 0; i < (both ? 16 : 8); i++)
	aarch64_set_vec_u8 (Vd, i, aarch64_get_reg_u8 (Rs, NO_SP));
      return status;

    case 2:
      for (i = 0; i < (both ? 8 : 4); i++)
	aarch64_set_vec_u16 (Vd, i, aarch64_get_reg_u16 (Rs, NO_SP));
      return status;

    case 4:
      for (i = 0; i < (both ? 4 : 2); i++)
	aarch64_set_vec_u32 (Vd, i, aarch64_get_reg_u32 (Rs, NO_SP));
      return status;

    case 8:
      if (!both)
	return_NYI;
      aarch64_set_vec_u64 (Vd, 0, aarch64_get_reg_u64 (Rs, NO_SP));
      return aarch64_set_vec_u64 (Vd, 1, aarch64_get_reg_u64 (Rs, NO_SP));

    default:
      return_NYI;
    }
}

static StatusCode
do_vec_UZP (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,24] = 00 1110
  // instr[23,22] = size: byte(00), half(01), word (10), long (11)
  // instr[21]    = 0
  // instr[20,16] = Vm
  // instr[15]    = 0
  // instr[14]    = lower (0) / upper (1)
  // instr[13,10] = 0110
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 0);
  NYI_assert (15, 15, 0);
  NYI_assert (13, 10, 6);

  int full = uimm (instr, 30, 30);
  int upper = uimm (instr, 14, 14);

  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);

  u_int64_t val_m1 = aarch64_get_vec_u64 (vm, 0);
  u_int64_t val_m2 = aarch64_get_vec_u64 (vm, 1);
  u_int64_t val_n1 = aarch64_get_vec_u64 (vn, 0);
  u_int64_t val_n2 = aarch64_get_vec_u64 (vn, 1);

  u_int64_t val1 = 0;
  u_int64_t val2 = 0;

  u_int64_t input1 = upper ? val_n1 : val_m1;
  u_int64_t input2 = upper ? val_n2 : val_m2;
  unsigned i;

  switch (uimm (instr, 23, 23))
    {
    case 0:
      for (i = 0; i < 8; i++)
	{
	  val1 |= (input1 >> (i * 8)) & (0xFFULL << (i * 8));
	  val2 |= (input2 >> (i * 8)) & (0xFFULL << (i * 8));
	}
      break;

    case 1:
      for (i = 0; i < 4; i++)
	{
	  val1 |= (input1 >> (i * 16)) & (0xFFFFULL << (i * 16));
	  val2 |= (input2 >> (i * 16)) & (0xFFFFULL << (i * 16));
	}
      break;

    case 2:
      val1 = ((input1 & 0xFFFFFFFF) | ((input1 >> 32) & 0xFFFFFFFF00000000ULL));
      val2 = ((input2 & 0xFFFFFFFF) | ((input2 >> 32) & 0xFFFFFFFF00000000ULL));
      
    case 3:
      val1 = input1;
      val2 = input2;
	   break;
    }

  aarch64_set_vec_u64 (vd, 0, val1);
  if (full)
    aarch64_set_vec_u64 (vd, 1, val2);
  return status;
}

static StatusCode
do_vec_ZIP (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,24] = 00 1110
  // instr[23,22] = size: byte(00), hald(01), word (10), long (11)
  // instr[21]    = 0
  // instr[20,16] = Vm
  // instr[15]    = 0
  // instr[14]    = lower (0) / upper (1)
  // instr[13,10] = 1110
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 0);
  NYI_assert (15, 15, 0);
  NYI_assert (13, 10, 0xE);

  int full = uimm (instr, 30, 30);
  int upper = uimm (instr, 14, 14);

  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);

  u_int64_t val_m1 = aarch64_get_vec_u64 (vm, 0);
  u_int64_t val_m2 = aarch64_get_vec_u64 (vm, 1);
  u_int64_t val_n1 = aarch64_get_vec_u64 (vn, 0);
  u_int64_t val_n2 = aarch64_get_vec_u64 (vn, 1);

  u_int64_t val1 = 0;
  u_int64_t val2 = 0;

  u_int64_t input1 = upper ? val_n1 : val_m1;
  u_int64_t input2 = upper ? val_n2 : val_m2;

  switch (uimm (instr, 23, 23))
    {
    case 0:
      val1 =
	  ((input1 <<  0) & (0xFF    <<  0)) | ((input2 << 8)  & (0xFF    << 8))
	| ((input1 <<  8) & (0xFF    << 16)) | ((input2 << 16) & (0xFF    << 24))
	| ((input1 << 16) & (0xFFULL << 32)) | ((input2 << 24) & (0xFFULL << 40))
	| ((input1 << 24) & (0xFFULL << 48)) | ((input2 << 32) & (0xFFULL << 56));

      val2 =
	  ((input1 >> 32) & (0xFF    <<  0)) | ((input2 >> 24) & (0xFF    << 8))
	| ((input1 >> 24) & (0xFF    << 16)) | ((input2 >> 16) & (0xFF    << 24)) 
	| ((input1 >> 16) & (0xFFULL << 32)) | ((input2 >>  8) & (0xFFULL << 40)) 
	| ((input1 >>  8) & (0xFFULL << 48)) | ((input2 >>  0) & (0xFFULL << 56));
      break;

    case 1:
      val1 =
	  ((input1 <<  0) & (0xFFFF    <<  0)) | ((input2 << 16) & (0xFFFF    << 16))
	| ((input1 << 16) & (0xFFFFULL << 32)) | ((input2 << 32) & (0xFFFFULL << 48));

      val2 =
	  ((input1 >> 32) & (0xFFFF    <<  0)) | ((input2 >> 16) & (0xFFFF    << 16))
	| ((input1 >> 16) & (0xFFFFULL << 32)) | ((input2 >>  0) & (0xFFFFULL << 48));
      break;

    case 2:
      val1 = (input1 & 0xFFFFFFFFULL) | (input2 << 32);
      val2 = (input2 & 0xFFFFFFFFULL) | (input1 << 32);
      break;

    case 3:
      val1 = input1;
      val2 = input2;
      break;
    }

  aarch64_set_vec_u64 (vd, 0, val1);
  if (full)
    aarch64_set_vec_u64 (vd, 1, val2);
  return status;
}

// floating point immediates are encoded in 8 bits
// fpimm[7] = sign bit
// fpimm[6:4] = signed exponent
// fpimm[3:0] = fraction (assuming leading 1)
// i.e. F = s * 1.f * 2^(e - b)

static float
fp_immediate_for_encoding_32 (u_int32_t imm8)
{
  float u;
  u_int32_t s, e, f, i;

  s = (imm8 >> 7) & 0x1;
  e = (imm8 >> 4) & 0x7;
  f = imm8 & 0xf;

  // The fp value is s * n/16 * 2r where n is 16+e.
  u = (16.0 + f) / 16.0;

  // N.B. exponent is signed.
  if (e < 4)
    {
      int epos = e;

      for (i = 0; i <= epos; i++)
	u *= 2.0;
    }
  else
    {
      int eneg = 7 - e;

      for (i = 0; i < eneg; i++)
	u /= 2.0;
    }

  if (s)
    u = - u;
  
  return u;
}

static double
fp_immediate_for_encoding_64 (u_int32_t imm8)
{
  double u;
  u_int32_t s, e, f, i;

  s = (imm8 >> 7) & 0x1;
  e = (imm8 >> 4) & 0x7;
  f = imm8 & 0xf;

  // The fp value is s * n/16 * 2r where n is 16+e.
  u = (16.0 + f) / 16.0;

  // N.B. exponent is signed.
  if (e < 4)
    {
      int epos = e;

      for (i = 0; i <= epos; i++)
	u *= 2.0;
    }
  else
    {
      int eneg = 7 - e;

      for (i = 0; i < eneg; i++)
	u /= 2.0;
    }

  if (s)
    u = - u;
  
  return u;
}

static StatusCode
do_vec_MOV_immediate (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29,19] = 00111100000
  // instr[18,16] = high 3 bits of uimm8
  // instr[15,12] = size & shift:
  //                              0000 => 32-bit
  //                              0010 => 32-bit + LSL#8
  //                              0100 => 32-bit + LSL#16
  //                              0110 => 32-bit + LSL#24
  //                              1010 => 16-bit + LSL#8
  //                              1000 => 16-bit
  //                              1101 => 32-bit + MSL#16
  //                              1100 => 32-bit + MSL#8  
  //                              1110 => 8-bit
  //                              1111 => double
  // instr[11,10] = 01
  // instr[9,5]   = low 5-bits of uimm8
  // instr[4,0]   = Vd

  NYI_assert (29, 19, 0x1E0);
  NYI_assert (11, 10, 1);

  int full     = uimm (instr, 30, 30);
  unsigned vd  = uimm (instr, 4, 0);
  unsigned val = uimm (instr, 18, 16) << 5 | uimm (instr, 9, 5);
  unsigned i;

  switch (uimm (instr, 15, 12))
    {
    case 0x0: /* 32-bit, no shift.  */
    case 0x2: /* 32-bit, shift by 8.  */
    case 0x4: /* 32-bit, shift by 16.  */
    case 0x6: /* 32-bit, shift by 24.  */
      val <<= (8 * uimm (instr, 14, 13));
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, val);
      return status;
      
    case 0xa: /* 16-bit, shift by 8.  */
      val <<= 8;
    case 0x8: /* 16-bit, no shift.  */
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_u16 (vd, i, val);
      
    case 0xd: /* 32-bit, mask shift by 16.  */
      val <<= 8;
      val |= 0xFF;
    case 0xc: /* 32-bit, mask shift by 8. */
      val <<= 8;
      val |= 0xFF;
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, val);
      return status;
      
    case 0xe: /* 8-bit, no shift.  */
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_u8 (vd, i, val);
      return status;

    case 0xf: /* FMOV Vs.{2|4}S, #fpimm.  */
      ; /* Necessary because a variable cannot be declared after a case label.  */
      float u = fp_immediate_for_encoding_32 (val);
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_float (vd, i, u);
      return status;

    default:
      return_NYI;
    }
}

static StatusCode
do_vec_MVNI (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29,19] = 10111100000
  // instr[18,16] = high 3 bits of uimm8
  // instr[15,12] = selector
  // instr[11,10] = 01
  // instr[9,5]   = low 5-bits of uimm8
  // instr[4,0]   = Vd

  NYI_assert (29, 19, 0x5E0);
  NYI_assert (11, 10, 1);

  int full     = uimm (instr, 30, 30);
  unsigned vd  = uimm (instr, 4, 0);
  unsigned val = uimm (instr, 18, 16) << 5 | uimm (instr, 9, 5);
  unsigned i;

  switch (uimm (instr, 15, 12))
    {
    case 0x0: /* 32-bit, no shift.  */
    case 0x2: /* 32-bit, shift by 8.  */
    case 0x4: /* 32-bit, shift by 16.  */
    case 0x6: /* 32-bit, shift by 24.  */
      val <<= (8 * uimm (instr, 14, 13));
      val = ~ val;
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, val);
      return status;

    case 0xa: /* 16-bit, 8 bit shift. */
      val <<= 8;
    case 0x8: /* 16-bit, no shift. */
      val = ~ val;
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_u16 (vd, i, val);
      return status;
      
    case 0xd: /* 32-bit, mask shift by 16.  */
      val <<= 8;
      val |= 0xFF;
    case 0xc: /* 32-bit, mask shift by 8. */
      val <<= 8;
      val |= 0xFF;
      val = ~ val;
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, val);
      return status;

    case 0xE: /* MOVI Dn, #mask64 */
      {
	u_int64_t mask = 0;

	for (i = 0; i < 8; i++)
	  if (val & (1 << i))
	    mask |= (0xF << (i * 4));
	aarch64_set_vec_u64 (vd, 0, mask);
	return aarch64_set_vec_u64 (vd, 1, 0);
      }

    case 0xf: /* FMOV Vd.2D, #fpimm.  */
      if (! full)
	return_UNALLOC;

      double u = fp_immediate_for_encoding_64 (val);
      aarch64_set_vec_double (vd, 0, u);
      aarch64_set_vec_double (vd, 1, u);
      return status;
      
    default:
      return_NYI;
    }
}

#define ABS(A) ((A) < 0 ? - (A) : (A))

static StatusCode
do_vec_ABS (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,24] = 00 1110
  // instr[23,22] = size: 00=> 8-bit, 01=> 16-bit, 10=> 32-bit, 11=> 64-bit
  // instr[21,10] = 10 0000 1011 10
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 10, 0x82E);

  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  unsigned i;

  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_s8 (vd, i, ABS (aarch64_get_vec_s8 (vn, i)));
      break;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_s16 (vd, i, ABS (aarch64_get_vec_s16 (vn, i)));
      break;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_s32 (vd, i, ABS (aarch64_get_vec_s32 (vn, i)));
      break;

    case 3:
      if (! full)
	return_NYI;
      for (i = 0; i < 2; i++)
	aarch64_set_vec_s64 (vd, i, ABS (aarch64_get_vec_s64 (vn, i)));
      break;
    }

  return status;    
}

static StatusCode
do_vec_ADDV (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29,24] = 00 1110
  // instr[23,22] = size: 00=> 8-bit, 01=> 16-bit, 10=> 32-bit, 11=> 64-bit
  // instr[21,10] = 11 0001 1011 10
  // instr[9,5]   = Vm
  // instr[4.0]   = Rd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 10, 0xC6E);

  unsigned vm = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  unsigned i;
  u_int64_t val = 0;
  int      full = uimm (instr, 30, 30);
  
  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	val += aarch64_get_vec_u8 (vm, i);
      return aarch64_set_reg_u64 (rd, NO_SP, val);

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	val += aarch64_get_vec_u16 (vm, i);
      return aarch64_set_reg_u64 (rd, NO_SP, val);

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	val += aarch64_get_vec_u32 (vm, i);
      return aarch64_set_reg_u64 (rd, NO_SP, val);

    case 3:
      if (! full)
	return_UNALLOC;
      val = aarch64_get_vec_u64 (vm, 0);
      val += aarch64_get_vec_u64 (vm, 1);
      return aarch64_set_reg_u64 (rd, NO_SP, val);

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

static StatusCode
do_vec_ins_2 (void)
{
  // instr[31,21] = 01001110000
  // instr[20,18] = size & element selector
  // instr[17,14] = 0000
  // instr[13]    = direction: to vec(0), from vec (1)
  // instr[12,10] = 111
  // instr[9,5]   = Vm
  // instr[4,0]   = Vd

  NYI_assert (31, 21, 0x270);
  NYI_assert (17, 14, 0);
  NYI_assert (12, 10, 7);

  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);

  if (uimm (instr, 13, 13) == 1)
    {
      if (uimm (instr, 18, 18) == 1)
	{
	  // 32-bit moves.
	  unsigned elem = uimm (instr, 20, 19);
	  aarch64_set_reg_u64 (vd, NO_SP,
			       aarch64_get_vec_u32 (vm, elem));
	}
      else
	{
	  // 64-bit moves.

	  if (uimm (instr, 19, 19) != 1)
	    return_NYI;
	  
	  unsigned elem = uimm (instr, 20, 20);
	  aarch64_set_reg_u64 (vd, NO_SP,
			       aarch64_get_vec_u64 (vm, elem));
	}
    }
  else
    {
      if (uimm (instr, 18, 18) == 1)
	{
	  // 32-bit moves.
	  unsigned elem = uimm (instr, 20, 19);
	  aarch64_set_vec_u32 (vd, elem,
			       aarch64_get_reg_u32 (vm, NO_SP));
	}
      else
	{
	  // 64-bit moves.

	  if (uimm (instr, 19, 19) != 1)
	    return_NYI;

	  unsigned elem = uimm (instr, 20, 20);
	  aarch64_set_vec_u64 (vd, elem,
			       aarch64_get_reg_u64 (vm, NO_SP));
	}
    }

  return status;
}

static StatusCode
do_vec_mull (void)
{
  // instr[31]    = 0
  // instr[30]    = lower(0)/upper(1) selector
  // instr[29]    = signed(0)/unsigned(1)
  // instr[28,24] = 0 1110
  // instr[23,22] = size: 8-bit (00), 16-bit (01), 32-bit (10)
  // instr[21]    = 1
  // instr[20,16] = Vm
  // instr[15,10] = 11 0000
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (28, 24, 0x0E);
  NYI_assert (15, 10, 0x30);

  int    unsign = uimm (instr, 29, 29);
  int    bias = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr,  9,  5);
  unsigned vd = uimm (instr,  4,  0);
  unsigned i;

  switch (uimm (instr, 23, 22))
    {
    case 0:
      if (bias)
	bias = 8;
      if (unsign)
	for (i = 0; i < 8; i++)
	  aarch64_set_vec_u16 (vd, i,
			       aarch64_get_vec_u8 (vn, i + bias)
			       * aarch64_get_vec_u8 (vm, i + bias));
      else
	for (i = 0; i < 8; i++)
	  aarch64_set_vec_s16 (vd, i,
			       aarch64_get_vec_s8 (vn, i + bias)
			       * aarch64_get_vec_s8 (vm, i + bias));	
      return status;

    case 1:
      if (bias)
	bias = 4;
      if (unsign)
	for (i = 0; i < 4; i++)
	  aarch64_set_vec_u32 (vd, i,
			       aarch64_get_vec_u16 (vn, i + bias)
			       * aarch64_get_vec_u16 (vm, i + bias));
      else
	for (i = 0; i < 4; i++)
	  aarch64_set_vec_s32 (vd, i,
			       aarch64_get_vec_s16 (vn, i + bias)
			       * aarch64_get_vec_s16 (vm, i + bias));
      return status;
      
    case 2:
      if (bias)
	bias = 2;
      if (unsign)
	for (i = 0; i < 2; i++)
	  aarch64_set_vec_u64 (vd, i,
			       (u_int64_t) aarch64_get_vec_u32 (vn, i + bias)
			       * (u_int64_t) aarch64_get_vec_u32 (vm, i + bias));
      else
	for (i = 0; i < 2; i++)
	  aarch64_set_vec_s64 (vd, i,
			       aarch64_get_vec_s32 (vn, i + bias)
			       * aarch64_get_vec_s32 (vm, i + bias));	
      return status;
      
    case 3:
    default:
      return_NYI;
    }
}

static StatusCode
do_vec_fadd (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,24] = 001110
  // instr[23]    = FADD(0)/FSUB(1)
  // instr[22]    = float (0)/double(1)
  // instr[21]    = 1
  // instr[20,16] = Vm
  // instr[15,10] = 110101
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x35);

  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  if (uimm (instr, 23, 23))
    {
      if (uimm (instr, 22, 22))
	{
	  if (! full)
	    return_NYI;

	  for (i = 0; i < 2; i++)
	    aarch64_set_vec_double (vd, i,
				    aarch64_get_vec_double (vn, i)
				    - aarch64_get_vec_double (vm, i));
	}
      else
	{
	  for (i = 0; i < (full ? 4 : 2); i++)
	    aarch64_set_vec_float (vd, i,
				   aarch64_get_vec_float (vn, i)
				   - aarch64_get_vec_float (vm, i));
	}
    }
  else
    {
      if (uimm (instr, 22, 22))
	{
	  if (! full)
	    return_NYI;

	  for (i = 0; i < 2; i++)
	    aarch64_set_vec_double (vd, i,
				    aarch64_get_vec_double (vm, i)
				    + aarch64_get_vec_double (vn, i));
	}
      else
	{
	  for (i = 0; i < (full ? 4 : 2); i++)
	    aarch64_set_vec_float (vd, i,
				   aarch64_get_vec_float (vm, i)
				   + aarch64_get_vec_float (vn, i));
	}
    }

  return status;
}

static StatusCode
do_vec_add (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29,24] = 001110
  // instr[23,22] = size: 00=> 8-bit, 01=> 16-bit, 10=> 32-bit, 11=> 64-bit
  // instr[21]    = 1
  // instr[20,16] = Vn
  // instr[15,10] = 100001
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x21);

  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);
  
  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_u8 (vd, i, aarch64_get_vec_u8 (vn, i) + aarch64_get_vec_u8 (vm, i));
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u16 (vn, i) + aarch64_get_vec_u16 (vm, i));
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u32 (vn, i) + aarch64_get_vec_u32 (vm, i));
      return status;

    case 3:
      if (! full)
	return_UNALLOC;
      aarch64_set_vec_u64 (vd, 0, aarch64_get_vec_u64 (vn, 0) + aarch64_get_vec_u64 (vm, 0));
      return aarch64_set_vec_u64 (vd, 1, aarch64_get_vec_u64 (vn, 1) + aarch64_get_vec_u64 (vm, 1));

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

static StatusCode
do_vec_mul (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29,24] = 00 1110
  // instr[23,22] = size: 00=> 8-bit, 01=> 16-bit, 10=> 32-bit
  // instr[21]    = 1
  // instr[20,16] = Vn
  // instr[15,10] = 10 0111
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x27);
  
  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);
  
  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	{
	  u_int16_t val = aarch64_get_vec_u8 (vn, i);
	  val *= aarch64_get_vec_u8 (vm, i);
      
	  aarch64_set_vec_u16 (vd, i, val);
	}
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	{
	  u_int32_t val = aarch64_get_vec_u16 (vn, i);
	  val *= aarch64_get_vec_u16 (vm, i);
      
	  aarch64_set_vec_u32 (vd, i, val);
	}
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	{
	  u_int64_t val = aarch64_get_vec_u32 (vn, i);
	  val *= aarch64_get_vec_u32 (vm, i);
      
	  aarch64_set_vec_u64 (vd, i, val);
	}
      return status;

    default:
    case 3:
      return_UNALLOC;
    }
}

static StatusCode
do_vec_MLA (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29,24] = 00 1110
  // instr[23,22] = size: 00=> 8-bit, 01=> 16-bit, 10=> 32-bit
  // instr[21]    = 1
  // instr[20,16] = Vn
  // instr[15,10] = 1001 01
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x25);

  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);
  
  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	{
	  u_int16_t val = aarch64_get_vec_u8 (vn, i);
	  val *= aarch64_get_vec_u8 (vm, i);
	  val += aarch64_get_vec_u8 (vd, i);
      
	  aarch64_set_vec_u16 (vd, i, val);
	}
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	{
	  u_int32_t val = aarch64_get_vec_u16 (vn, i);
	  val *= aarch64_get_vec_u16 (vm, i);
	  val += aarch64_get_vec_u16 (vd, i);
      
	  aarch64_set_vec_u32 (vd, i, val);
	}
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	{
	  u_int64_t val = aarch64_get_vec_u32 (vn, i);
	  val *= aarch64_get_vec_u32 (vm, i);
	  val += aarch64_get_vec_u32 (vd, i);
      
	  aarch64_set_vec_u64 (vd, i, val);
	}
      return status;

    default:
    case 3:
      return_UNALLOC;
    }
}

static float
fmaxnm (float a, float b)
{
  if (fpclassify (a) == FP_NORMAL)
    {
      if (fpclassify (b) == FP_NORMAL)
	return a > b ? a : b;
      return a;
    }
  else if (fpclassify (b) == FP_NORMAL)
    return b;
  return a;
}

static float
fminnm (float a, float b)
{
  if (fpclassify (a) == FP_NORMAL)
    {
      if (fpclassify (b) == FP_NORMAL)
	return a < b ? a : b;
      return a;
    }
  else if (fpclassify (b) == FP_NORMAL)
    return b;
  return a;
}

static double
dmaxnm (double a, double b)
{
  if (fpclassify (a) == FP_NORMAL)
    {
      if (fpclassify (b) == FP_NORMAL)
	return a > b ? a : b;
      return a;
    }
  else if (fpclassify (b) == FP_NORMAL)
    return b;
  return a;
}

static double
dminnm (double a, double b)
{
  if (fpclassify (a) == FP_NORMAL)
    {
      if (fpclassify (b) == FP_NORMAL)
	return a < b ? a : b;
      return a;
    }
  else if (fpclassify (b) == FP_NORMAL)
    return b;
  return a;
}

static StatusCode
do_vec_FminmaxNMP (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,24] = 10 1110
  // instr[23]    = max(0)/min(1)
  // instr[22]    = float (0)/double (1)
  // instr[21]    = 1
  // instr[20,16] = Vn
  // instr[15,10] = 1100 01
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x2E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x31);
  
  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  int      full = uimm (instr, 30, 30);

  if (uimm (instr, 22, 22))
    {
      double (* fn)(double, double) = uimm (instr, 23, 23) ? dminnm : dmaxnm;

      if (! full)
	return_NYI;
      aarch64_set_vec_double (vd, 0, fn (aarch64_get_vec_double (vn, 0),
					 aarch64_get_vec_double (vn, 1)));
      aarch64_set_vec_double (vd, 0, fn (aarch64_get_vec_double (vm, 0),
					 aarch64_get_vec_double (vm, 1)));
    }
  else
    {
      float (* fn)(float, float) = uimm (instr, 23, 23) ? fminnm : fmaxnm;

      aarch64_set_vec_float (vd, 0, fn (aarch64_get_vec_float (vn, 0),
					aarch64_get_vec_float (vn, 1)));
      if (full)
	aarch64_set_vec_float (vd, 1, fn (aarch64_get_vec_float (vn, 2),
					  aarch64_get_vec_float (vn, 3)));

      aarch64_set_vec_float (vd, (full ? 2 : 1), fn (aarch64_get_vec_float (vm, 0),
						     aarch64_get_vec_float (vm, 1)));
      if (full)
	aarch64_set_vec_float (vd, 3, fn (aarch64_get_vec_float (vm, 2),
					  aarch64_get_vec_float (vm, 3)));
    }

  return status;
}

static StatusCode
do_vec_AND (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,21] = 001110001
  // instr[20,16] = Vm
  // instr[15,10] = 000111
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 21, 0x071);
  NYI_assert (15, 10, 0x07);

  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  for (i = 0; i < (full ? 4 : 2); i++)
    aarch64_set_vec_u32 (vd, i,
			 aarch64_get_vec_u32 (vn, i)
			 & aarch64_get_vec_u32 (vm, i));

  return status;
}

static StatusCode
do_vec_BSL (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,21] = 101110011
  // instr[20,16] = Vm
  // instr[15,10] = 000111
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 21, 0x173);
  NYI_assert (15, 10, 0x07);

  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  for (i = 0; i < (full ? 16 : 8); i++)
    aarch64_set_vec_u8 (vd, i, 
			(     aarch64_get_vec_u8 (vd, i)  & aarch64_get_vec_u8 (vn, i))
			| ((~ aarch64_get_vec_u8 (vd, i)) & aarch64_get_vec_u8 (vm, i)));
  return status;
}

static StatusCode
do_vec_EOR (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,21] = 10 1110 001
  // instr[20,16] = Vm
  // instr[15,10] = 000111
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 21, 0x171);
  NYI_assert (15, 10, 0x07);

  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  for (i = 0; i < (full ? 4 : 2); i++)
    aarch64_set_vec_u32 (vd, i,
			 aarch64_get_vec_u32 (vn, i)
			 ^ aarch64_get_vec_u32 (vm, i));

  return status;
}

static StatusCode
do_vec_bit (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,23] = 10 1110 1
  // instr[22]    = BIT (0) / BIF (1)
  // instr[21]    = 1
  // instr[20,16] = Vm
  // instr[15,10] = 0001 11
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 23, 0x5D);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x07);

  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  unsigned test_false = uimm (instr, 22, 22);
  unsigned i;

  if (test_false)
    {
      for (i = 0; i < (full ? 16 : 8); i++)
	if (aarch64_get_vec_u32 (vn, i) == 0)
	  aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u32 (vm, i));
    }
  else
    {
      for (i = 0; i < (full ? 16 : 8); i++)
	if (aarch64_get_vec_u32 (vn, i) != 0)
	  aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u32 (vm, i));
    }

  return status;
}

static StatusCode
do_vec_ORN (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,21] = 00 1110 111
  // instr[20,16] = Vm
  // instr[15,10] = 00 0111
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 21, 0x077);
  NYI_assert (15, 10, 0x07);
  
  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  for (i = 0; i < (full ? 16 : 8); i++)
    aarch64_set_vec_u8 (vd, i,
			aarch64_get_vec_u8 (vn, i)
			| ~ aarch64_get_vec_u8 (vm, i));
  return status;
}

static StatusCode
do_vec_ORR (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,21] = 00 1110 101
  // instr[20,16] = Vm
  // instr[15,10] = 0001 11
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 21, 0x075);
  NYI_assert (15, 10, 0x07);
  
  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  for (i = 0; i < (full ? 16 : 8); i++)
    aarch64_set_vec_u8 (vd, i,
			aarch64_get_vec_u8 (vn, i)
			| aarch64_get_vec_u8 (vm, i));
  return status;
}

static StatusCode
do_vec_XTN (void)
{
  // instr[31]    = 0
  // instr[30]    = first part (0)/ second part (1)
  // instr[29,24] = 00 1110
  // instr[23,22] = size: byte(00), half(01), word (10)
  // instr[21,10] = 1000 0100 1010
  // instr[9,5]   = Vs
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 10, 0x84A);

  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned bias = uimm (instr, 30, 30);
  unsigned i;
  
  switch (uimm (instr, 23, 22))
    {
    case 0:
      if (bias)
	for (i = 0; i < 8; i++)
	  aarch64_set_vec_u8 (vd, i + 8, aarch64_get_vec_u16 (vs, i) >> 8);
      else
	for (i = 0; i < 8; i++)
	  aarch64_set_vec_u8 (vd, i, aarch64_get_vec_u16 (vs, i));
      return status;

    case 1:
      if (bias)
	for (i = 0; i < 4; i++)
	  aarch64_set_vec_u16 (vd, i + 4, aarch64_get_vec_u32 (vs, i) >> 16);
      else
	for (i = 0; i < 4; i++)
	  aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u32 (vs, i));
      return status;

    case 2:
      if (bias)
	for (i = 0; i < 2; i++)
	  aarch64_set_vec_u32 (vd, i + 4, aarch64_get_vec_u64 (vs, i) >> 32);
      else
	for (i = 0; i < 2; i++)
	  aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u64 (vs, i));
      return status;

    default:
      return_UNALLOC;
    }
}

#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) < (B) ? (A) : (B))

static StatusCode
do_vec_maxv (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29]    = signed (0)/unsigned(1)
  // instr[28,24] = 0 1110
  // instr[23,22] = size: byte(00), half(01), word (10)
  // instr[21]    = 1
  // instr[20,17] = 1 000
  // instr[16]    = max(0)/min(1)
  // instr[15,10] = 1010 10
  // instr[9,5]   = V source
  // instr[4.0]   = R dest

  NYI_assert (28, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (20, 17, 8);
  NYI_assert (15, 10, 0x2A);

  unsigned vs = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  unsigned i;

  switch ((uimm (instr, 29, 29) << 1) | uimm (instr, 16, 16))
    {
    case 0: // SMAXV
       {
	int64_t smax;
	switch (uimm (instr, 23, 22))
	  {
	  case 0:
	    smax = aarch64_get_vec_s8 (vs, 0);
	    for (i = 1; i < (full ? 16 : 8); i++)
	      smax = MAX (smax, aarch64_get_vec_s8 (vs, i));
	    break;
	  case 1:
	    smax = aarch64_get_vec_s16 (vs, 0);
	    for (i = 1; i < (full ? 8 : 4); i++)
	      smax = MAX (smax, aarch64_get_vec_s16 (vs, i));
	    break;
	  case 2:
	    smax = aarch64_get_vec_s32 (vs, 0);
	    for (i = 1; i < (full ? 4 : 2); i++)
	      smax = MAX (smax, aarch64_get_vec_s32 (vs, i));
	    break;
	  default:
	  case 3:
	    return_UNALLOC;
	  }
	return aarch64_set_reg_s64 (rd, NO_SP, smax);
      }
      
    case 1: // SMINV
      {
	int64_t smin;
	switch (uimm (instr, 23, 22))
	  {
	  case 0:
	    smin = aarch64_get_vec_s8 (vs, 0);
	    for (i = 1; i < (full ? 16 : 8); i++)
	      smin = MIN (smin, aarch64_get_vec_s8 (vs, i));
	    break;
	  case 1:
	    smin = aarch64_get_vec_s16 (vs, 0);
	    for (i = 1; i < (full ? 8 : 4); i++)
	      smin = MIN (smin, aarch64_get_vec_s16 (vs, i));
	    break;
	  case 2:
	    smin = aarch64_get_vec_s32 (vs, 0);
	    for (i = 1; i < (full ? 4 : 2); i++)
	      smin = MIN (smin, aarch64_get_vec_s32 (vs, i));
	    break;
	  default:
	  case 3:
	    return_UNALLOC;
	  }
	return aarch64_set_reg_s64 (rd, NO_SP, smin);
      }

    case 2: // UMAXV
      {
	u_int64_t umax;
	switch (uimm (instr, 23, 22))
	  {
	  case 0:
	    umax = aarch64_get_vec_u8 (vs, 0);
	    for (i = 1; i < (full ? 16 : 8); i++)
	      umax = MAX (umax, aarch64_get_vec_u8 (vs, i));
	    break;
	  case 1:
	    umax = aarch64_get_vec_u16 (vs, 0);
	    for (i = 1; i < (full ? 8 : 4); i++)
	      umax = MAX (umax, aarch64_get_vec_u16 (vs, i));
	    break;
	  case 2:
	    umax = aarch64_get_vec_u32 (vs, 0);
	    for (i = 1; i < (full ? 4 : 2); i++)
	      umax = MAX (umax, aarch64_get_vec_u32 (vs, i));
	    break;
	  default:
	  case 3:
	    return_UNALLOC;
	  }
	return aarch64_set_reg_u64 (rd, NO_SP, umax);
      }
      
    case 3: // UMINV
      {
	u_int64_t umin;
	switch (uimm (instr, 23, 22))
	  {
	  case 0:
	    umin = aarch64_get_vec_u8 (vs, 0);
	    for (i = 1; i < (full ? 16 : 8); i++)
	      umin = MIN (umin, aarch64_get_vec_u8 (vs, i));
	    break;
	  case 1:
	    umin = aarch64_get_vec_u16 (vs, 0);
	    for (i = 1; i < (full ? 8 : 4); i++)
	      umin = MIN (umin, aarch64_get_vec_u16 (vs, i));
	    break;
	  case 2:
	    umin = aarch64_get_vec_u32 (vs, 0);
	    for (i = 1; i < (full ? 4 : 2); i++)
	      umin = MIN (umin, aarch64_get_vec_u32 (vs, i));
	    break;
	  default:
	  case 3:
	    return_UNALLOC;
	  }
	return aarch64_set_reg_u64 (rd, NO_SP, umin);
      }

    default:
      return_UNALLOC;
    }
}

static StatusCode
do_vec_fminmaxV (void)
{
  // instr[31,24] = 0110 1110
  // instr[23]    = max(0)/min(1)
  // instr[22,14] = 011 0000 11
  // instr[13,12] = nm(00)/normal(11)
  // instr[11,10] = 10
  // instr[9,5]   = V source
  // instr[4.0]   = R dest

  NYI_assert (31, 24, 0x6E);
  NYI_assert (22, 14, 0x0C3);
  NYI_assert (11, 10, 2);

  unsigned vs = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  unsigned i;
  float res   = aarch64_get_vec_float (vs, 0);

  if (uimm (instr, 23, 23))
    {
      switch (uimm (instr, 13, 12))
	{
	case 0: // FMNINNMV
	  for (i = 1; i < 4; i++)
	    res = fminnm (res, aarch64_get_vec_float (vs, i));
	  break;

	case 3: // FMINV
	  for (i = 1; i < 4; i++)
	    res = MIN (res, aarch64_get_vec_float (vs, i));
	  break;

	default:
	  return_NYI;
	}
    }
  else
    {
      switch (uimm (instr, 13, 12))
	{
	case 0: // FMNAXNMV
	  for (i = 1; i < 4; i++)
	    res = fmaxnm (res, aarch64_get_vec_float (vs, i));
	  break;

	case 3: // FMAXV
	  for (i = 1; i < 4; i++)
	    res = MAX (res, aarch64_get_vec_float (vs, i));
	  break;

	default:
	  return_NYI;
	}
    }

  return aarch64_set_FP_float (rd, res);
}

static StatusCode
do_vec_Fminmax (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,24] = 00 1110
  // instr[23]    = max(0)/min(1)
  // instr[22]    = float(0)/double(1)
  // instr[21]    = 1
  // instr[20,16] = Vm
  // instr[15,14] = 11
  // instr[13,12] = nm(00)/normal(11)
  // instr[11,10] = 01
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 14, 3);
  NYI_assert (11, 10, 1);

  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  unsigned min = uimm (instr, 23, 23);
  unsigned i;

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_NYI;

      double (* func)(double, double);

      if (uimm (instr, 13, 12) == 0)
	func = min ? dminnm : dmaxnm;
      else if (uimm (instr, 13, 12) == 3)
	func = min ? fmin : fmax;
      else
	return_NYI;
      
      for (i = 0; i < 2; i++)
	aarch64_set_vec_double (vd, i, func (aarch64_get_vec_double (vn, i),
					     aarch64_get_vec_double (vm, i)));
    }
  else
    {
      float (* func)(float, float);

      if (uimm (instr, 13, 12) == 0)
	func = min ? fminnm : fmaxnm;
      else if (uimm (instr, 13, 12) == 3)
	func = min ? fminf : fmaxf;
      else
	return_NYI;
      
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_float (vd, i, func (aarch64_get_vec_float (vn, i),
					    aarch64_get_vec_float (vm, i)));
    }
  return status;
}

static StatusCode
do_vec_SCVTF (void)
{
  // instr[31]    = 0
  // instr[30]    = Q
  // instr[29,23] = 00 1110 0
  // instr[22]    = float(0)/double(1)
  // instr[21,10] = 10 0001 1101 10
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  NYI_assert (29, 23, 0x1C);
  NYI_assert (21, 10, 0x876);

  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  unsigned size = uimm (instr, 22, 22);
  unsigned i;

  if (size)
    {
      if (! full)
	return_UNALLOC;

      for (i = 0; i < 2; i++)
	{
	  double val = (double) aarch64_get_vec_u64 (vn, i);
	  aarch64_set_vec_double (vd, i, val);
	}
    }
  else
    {
      for (i = 0; i < (full ? 4 : 2); i++)
	{
	  float val = (float) aarch64_get_vec_u32 (vn, i);
	  aarch64_set_vec_float (vd, i, val);
	}
    }
  return status;
}

#define VEC_CMP(SOURCE, CMP)						\
  do									\
    {									\
      switch (size)							\
	{								\
	case 0:								\
	  for (i = 0; i < (full ? 16 : 8); i++)				\
	    aarch64_set_vec_u8 (vd, i, aarch64_get_vec_##SOURCE##8 (vn, i) CMP \
				aarch64_get_vec_##SOURCE##8 (vm, i) ? -1 : 0); \
	  return status;						\
	case 1:								\
	  for (i = 0; i < (full ? 8 : 4); i++)				\
	    aarch64_set_vec_u16 (vd, i, aarch64_get_vec_##SOURCE##16 (vn, i) CMP \
				 aarch64_get_vec_##SOURCE##16 (vm, i) ? -1 : 0); \
	  return status;						\
	case 2:								\
	  for (i = 0; i < (full ? 4 : 2); i++)				\
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_##SOURCE##32 (vn, i) CMP \
				 aarch64_get_vec_##SOURCE##32 (vm, i) ? -1 : 0); \
	  return status;						\
	case 3:								\
	  if (! full)							\
	    return_UNALLOC;						\
	  for (i = 0; i < 2; i++)					\
	    aarch64_set_vec_u64 (vd, i, aarch64_get_vec_##SOURCE##64 (vn, i) CMP \
				 aarch64_get_vec_##SOURCE##64 (vm, i) ? -1ULL : 0); \
	  return status;						\
	default:							\
	return_UNALLOC;							\
	}								\
    }									\
  while (0)

#define VEC_CMP0(SOURCE, CMP)						\
  do									\
    {									\
      switch (size)							\
	{								\
	case 0:								\
	  for (i = 0; i < (full ? 16 : 8); i++)				\
	    aarch64_set_vec_u8 (vd, i, aarch64_get_vec_##SOURCE##8 (vn, i) CMP \
				0 ? -1 : 0); \
	  return status;						\
	case 1:								\
	  for (i = 0; i < (full ? 8 : 4); i++)				\
	    aarch64_set_vec_u16 (vd, i, aarch64_get_vec_##SOURCE##16 (vn, i) CMP \
				 0 ? -1 : 0); \
	  return status;						\
	case 2:								\
	  for (i = 0; i < (full ? 4 : 2); i++)				\
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_##SOURCE##32 (vn, i) CMP \
				 0 ? -1 : 0); \
	  return status;						\
	case 3:								\
	  if (! full)							\
	    return_UNALLOC;						\
	  for (i = 0; i < 2; i++)					\
	    aarch64_set_vec_u64 (vd, i, aarch64_get_vec_##SOURCE##64 (vn, i) CMP \
				 0 ? -1ULL : 0); \
	  return status;						\
	default:							\
	return_UNALLOC;							\
	}								\
    }									\
  while (0)

#define VEC_FCMP0(CMP)							\
  do									\
    {									\
      if (vm != 0)							\
	return_NYI;							\
      if (uimm (instr, 22, 22))						\
	{								\
	  if (! full)							\
	    return_NYI;							\
	  for (i = 0; i < 2; i++)					\
	    aarch64_set_vec_u64 (vd, i, aarch64_get_vec_double (vn, i) CMP 0.0 ? -1 : 0); \
	}								\
      else								\
	{								\
	  for (i = 0; i < (full ? 4 : 2); i++)				\
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_float (vn, i) CMP 0.0 ? -1 : 0); \
	}								\
      return status;							\
    }									\
  while (0)

#define VEC_FCMP(CMP)							\
  do									\
    {									\
      if (uimm (instr, 22, 22))						\
	{								\
	  if (! full)							\
	    return_NYI;							\
	  for (i = 0; i < 2; i++)					\
	    aarch64_set_vec_u64 (vd, i, aarch64_get_vec_double (vn, i) CMP \
				 aarch64_get_vec_double (vm, i) ? -1 : 0); \
	}								\
      else								\
	{								\
	  for (i = 0; i < (full ? 4 : 2); i++)				\
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_float (vn, i) CMP \
				 aarch64_get_vec_float (vm, i) ? -1 : 0); \
	}								\
      return status;							\
    }									\
  while (0)

static StatusCode
do_vec_compare (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29]    = part-of-comparison-type
  // instr[28,24] = 0 1110
  // instr[23,22] = size of integer compares: byte(00), half(01), word (10), long (11)
  //                type of float compares: single (-0) / double (-1)
  // instr[21]    = 1
  // instr[20,16] = Vm or 00000 (compare vs 0)
  // instr[15,10] = part-of-comparison-type
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (28, 24, 0x0E);
  NYI_assert (21, 21, 1);

  int full = uimm (instr, 30, 30);
  int size = uimm (instr, 23, 22);
  unsigned vm = uimm (instr, 20, 16);

  if ((uimm (instr, 11, 11) && uimm (instr, 14, 14))
      || ((uimm (instr, 11, 11) == 0 && uimm (instr, 10, 10) == 0))
      )
    {
      // A compare vs 0.
      if (vm != 0)
	{
	  if (uimm (instr, 15, 10) == 0x2A)
	    return do_vec_maxv ();

	  if (uimm (instr, 15, 10) == 0x32
	      || uimm (instr, 15, 10) == 0x3E)
	    return do_vec_fminmaxV ();

	  if (uimm (instr, 29, 23) == 0x1C
	      && uimm (instr, 21, 10) == 0x876)
	    return do_vec_SCVTF ();

	  return_NYI;
	}
    }
  
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  if (uimm (instr, 14, 14))
    {
      // A floating point compare.
      NYI_assert (15, 15, 1);

      unsigned decode = (uimm (instr, 29, 29) << 5) | (uimm (instr, 23, 23) << 4) | uimm (instr, 13, 10);

      switch (decode)
	{
	case /* 0b010010: GT#0 */ 0x12: VEC_FCMP0 (>);
	case /* 0b110010: GE#0 */ 0x32: VEC_FCMP0 (>=);
	case /* 0b010110: EQ#0 */ 0x16: VEC_FCMP0 (==);
	case /* 0b110110: LE#0 */ 0x36: VEC_FCMP0 (<=);
	case /* 0b011010: LT#0 */ 0x1A: VEC_FCMP0 (<);
	case /* 0b111001: GT */   0x39: VEC_FCMP  (>);
	case /* 0b101001: GE */   0x29: VEC_FCMP  (>=);
	case /* 0b001001: EQ */   0x09: VEC_FCMP  (==);

	default:
	  return_NYI;
	}
    }
  else
    {
      unsigned decode = (uimm (instr, 29, 29) << 6) | uimm (instr, 15, 10);

      switch (decode)
	{
	case 0x0D: /* 0001101 GT */     VEC_CMP  (s, > );
	case 0x0F: /* 0001111 GE */     VEC_CMP  (s, >= );
	case 0x22: /* 0100010 GT #0 */  VEC_CMP0 (s, > );
	case 0x26: /* 0100110 EQ #0 */  VEC_CMP0 (s, == );
	case 0x2A: /* 0101010 LT #0 */  VEC_CMP0 (s, < );
	case 0x4D: /* 1001101 HI */     VEC_CMP  (u, > );
	case 0x4F: /* 1001111 HS */     VEC_CMP  (u, >= );
	case 0x62: /* 1100010 GE #0 */  VEC_CMP0 (s, >= );
	case 0x63: /* 1100011 EQ */     VEC_CMP  (u, == );
	case 0x66: /* 1100110 LE #0 */  VEC_CMP0 (s, <= );
	default:
	  if (vm != 0)
	    return do_vec_maxv ();
	  return_NYI;
	}
    }
}

static StatusCode
do_vec_SSHL (void)
{
  // instr[31]    = 0
  // instr[30]    = first part (0)/ second part (1)
  // instr[29,24] = 00 1110
  // instr[23,22] = size: byte(00), half(01), word (10), long (11)
  // instr[21]    = 1
  // instr[20,16] = Vm
  // instr[15,10] = 0100 01
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x11);

  unsigned full = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  // FIXME: What is a signed shift left in this context ?
  
  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_s8 (vd, i, aarch64_get_vec_s8 (vn, i)
			    << aarch64_get_vec_s8 (vm, i));
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_s16 (vd, i, aarch64_get_vec_s16 (vn, i)
			    << aarch64_get_vec_s16 (vm, i));
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_s32 (vd, i, aarch64_get_vec_s32 (vn, i)
			    << aarch64_get_vec_s32 (vm, i));
      return status;

    case 3:
      if (! full)
	return_UNALLOC;
      for (i = 0; i < 2; i++)
	aarch64_set_vec_s64 (vd, i, aarch64_get_vec_s64 (vn, i)
			    << aarch64_get_vec_s64 (vm, i));
      return status;

    default:
      return_NYI;
    }
}

static StatusCode
do_vec_USHL (void)
{
  // instr[31]    = 0
  // instr[30]    = first part (0)/ second part (1)
  // instr[29,24] = 10 1110
  // instr[23,22] = size: byte(00), half(01), word (10), long (11)
  // instr[21]    = 1
  // instr[20,16] = Vm
  // instr[15,10] = 0100 01
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x2E);
  NYI_assert (15, 10, 0x11);

  unsigned full = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  
  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_u8 (vd, i, aarch64_get_vec_u8 (vn, i)
			    << aarch64_get_vec_u8 (vm, i));
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u16 (vn, i)
			    << aarch64_get_vec_u16 (vm, i));
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u32 (vn, i)
			    << aarch64_get_vec_u32 (vm, i));
      return status;

    case 3:
      if (! full)
	return_UNALLOC;
      for (i = 0; i < 2; i++)
	aarch64_set_vec_u64 (vd, i, aarch64_get_vec_u64 (vn, i)
			    << aarch64_get_vec_u64 (vm, i));
      return status;

    default:
      return_NYI;
    }
}

static StatusCode
do_vec_FMLA (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29,23] = 0011100
  // instr[22]    = size: 0=>float, 1=>double
  // instr[21]    = 1
  // instr[20,16] = Vn
  // instr[15,10] = 1100 11
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (29, 23, 0x1C);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x33);
  
  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);
  
  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_UNALLOC;
      for (i = 0; i < 2; i++)
	aarch64_set_vec_double (vd, i,
				aarch64_get_vec_double (vn, i) *
				aarch64_get_vec_double (vm, i) +
				aarch64_get_vec_double (vd, i));
    }
  else
    {
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_float (vd, i,
			       aarch64_get_vec_float (vn, i) *
			       aarch64_get_vec_float (vm, i) +
			       aarch64_get_vec_float (vd, i));
    }
  return status;
}

static StatusCode
do_vec_max (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29]    = SMAX (0) / UMAX (1)
  // instr[28,24] = 0 1110
  // instr[23,22] = size: 00=> 8-bit, 01=> 16-bit, 10=> 32-bit
  // instr[21]    = 1
  // instr[20,16] = Vn
  // instr[15,10] = 0110 01
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (28, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x19);
  
  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  if (uimm (instr, 29, 29))
    {
      switch (uimm (instr, 23, 22))
	{
	case 0:
	  for (i = 0; i < (full ? 16 : 8); i++)
	    aarch64_set_vec_u8 (vd, i, aarch64_get_vec_u8 (vn, i) > aarch64_get_vec_u8 (vm, i)
				? aarch64_get_vec_u8 (vn, i) : aarch64_get_vec_u8 (vm, i));
	  return status;

	case 1:
	  for (i = 0; i < (full ? 8 : 4); i++)
	    aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u16 (vn, i) > aarch64_get_vec_u16 (vm, i)
				 ? aarch64_get_vec_u16 (vn, i) : aarch64_get_vec_u16 (vm, i));
	  return status;

	case 2:
	  for (i = 0; i < (full ? 4 : 2); i++)
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u32 (vn, i) > aarch64_get_vec_u32 (vm, i)
				 ? aarch64_get_vec_u32 (vn, i) : aarch64_get_vec_u32 (vm, i));
	  return status;

	default:
	case 3:
	  return_UNALLOC;
	}
    }
  else
    {
      switch (uimm (instr, 23, 22))
	{
	case 0:
	  for (i = 0; i < (full ? 16 : 8); i++)
	    aarch64_set_vec_s8 (vd, i, aarch64_get_vec_s8 (vn, i) > aarch64_get_vec_s8 (vm, i)
				? aarch64_get_vec_s8 (vn, i) : aarch64_get_vec_s8 (vm, i));
	  return status;

	case 1:
	  for (i = 0; i < (full ? 8 : 4); i++)
	    aarch64_set_vec_s16 (vd, i, aarch64_get_vec_s16 (vn, i) > aarch64_get_vec_s16 (vm, i)
				 ? aarch64_get_vec_s16 (vn, i) : aarch64_get_vec_s16 (vm, i));
	  return status;

	case 2:
	  for (i = 0; i < (full ? 4 : 2); i++)
	    aarch64_set_vec_s32 (vd, i, aarch64_get_vec_s32 (vn, i) > aarch64_get_vec_s32 (vm, i)
				 ? aarch64_get_vec_s32 (vn, i) : aarch64_get_vec_s32 (vm, i));
	  return status;

	default:
	case 3:
	  return_UNALLOC;
	}
    }
}

static StatusCode
do_vec_min (void)
{
  // instr[31]    = 0
  // instr[30]    = full/half selector
  // instr[29]    = SMIN (0) / UMIN (1)
  // instr[28,24] = 0 1110
  // instr[23,22] = size: 00=> 8-bit, 01=> 16-bit, 10=> 32-bit
  // instr[21]    = 1
  // instr[20,16] = Vn
  // instr[15,10] = 0110 11
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (28, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x1B);
  
  unsigned vn = uimm (instr, 20, 16);
  unsigned vm = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  if (uimm (instr, 29, 29))
    {
      switch (uimm (instr, 23, 22))
	{
	case 0:
	  for (i = 0; i < (full ? 16 : 8); i++)
	    aarch64_set_vec_u8 (vd, i, aarch64_get_vec_u8 (vn, i) < aarch64_get_vec_u8 (vm, i)
				? aarch64_get_vec_u8 (vn, i) : aarch64_get_vec_u8 (vm, i));
	  return status;

	case 1:
	  for (i = 0; i < (full ? 8 : 4); i++)
	    aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u16 (vn, i) < aarch64_get_vec_u16 (vm, i)
				 ? aarch64_get_vec_u16 (vn, i) : aarch64_get_vec_u16 (vm, i));
	  return status;

	case 2:
	  for (i = 0; i < (full ? 4 : 2); i++)
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u32 (vn, i) < aarch64_get_vec_u32 (vm, i)
				 ? aarch64_get_vec_u32 (vn, i) : aarch64_get_vec_u32 (vm, i));
	  return status;

	default:
	case 3:
	  return_UNALLOC;
	}
    }
  else
    {
      switch (uimm (instr, 23, 22))
	{
	case 0:
	  for (i = 0; i < (full ? 16 : 8); i++)
	    aarch64_set_vec_s8 (vd, i, aarch64_get_vec_s8 (vn, i) < aarch64_get_vec_s8 (vm, i)
				? aarch64_get_vec_s8 (vn, i) : aarch64_get_vec_s8 (vm, i));
	  return status;

	case 1:
	  for (i = 0; i < (full ? 8 : 4); i++)
	    aarch64_set_vec_s16 (vd, i, aarch64_get_vec_s16 (vn, i) < aarch64_get_vec_s16 (vm, i)
				 ? aarch64_get_vec_s16 (vn, i) : aarch64_get_vec_s16 (vm, i));
	  return status;

	case 2:
	  for (i = 0; i < (full ? 4 : 2); i++)
	    aarch64_set_vec_s32 (vd, i, aarch64_get_vec_s32 (vn, i) < aarch64_get_vec_s32 (vm, i)
				 ? aarch64_get_vec_s32 (vn, i) : aarch64_get_vec_s32 (vm, i));
	  return status;

	default:
	case 3:
	  return_UNALLOC;
	}
    }
}

static StatusCode
do_vec_sub_long (void)
{
  // instr[31]    = 0
  // instr[30]    = lower (0) / upper (1)
  // instr[29]    = signed (0) / unsigned (1)
  // instr[28,24] = 0 1110
  // instr[23,22] = size: bytes (00), half (01), word (10)
  // instr[21]    = 1
  // insrt[20,16] = Vm
  // instr[15,10] = 0010 00
  // instr[9,5]   = Vn
  // instr[4,0]   = V dest

  NYI_assert (28, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x08);

  unsigned size = uimm (instr, 23, 22);
  if (size == 3)
    return_UNALLOC;

  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned bias = 0;
  unsigned i;

  switch (uimm (instr, 30, 29))
    {
    case 2: // SSUBL2
      bias = 2;
    case 0: // SSUBL
      switch (size)
	{
	case 0:
	  bias *= 3;
	  for (i = 0; i < 8; i++)
	    aarch64_set_vec_s16 (vd, i, aarch64_get_vec_s8 (vn, i + bias)
				 - aarch64_get_vec_s8 (vm, i + bias));
	  break;

	case 1:
	  bias *= 2;
	  for (i = 0; i < 4; i++)
	    aarch64_set_vec_s32 (vd, i, aarch64_get_vec_s16 (vn, i + bias)
				 - aarch64_get_vec_s16 (vm, i + bias));
	  break;

	case 2:
	  for (i = 0; i < 2; i++)
	    aarch64_set_vec_s64 (vd, i, aarch64_get_vec_s32 (vn, i + bias)
				 - aarch64_get_vec_s32 (vm, i + bias));
	  break;

	default:
	  return_UNALLOC;
	}
      break;

    case 3: // USUBL2
      bias = 2;
    case 1: // USUBL
      switch (size)
	{
	case 0:
	  bias *= 3;
	  for (i = 0; i < 8; i++)
	    aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u8 (vn, i + bias)
				 - aarch64_get_vec_u8 (vm, i + bias));
	  break;

	case 1:
	  bias *= 2;
	  for (i = 0; i < 4; i++)
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u16 (vn, i + bias)
				 - aarch64_get_vec_u16 (vm, i + bias));
	  break;

	case 2:
	  for (i = 0; i < 2; i++)
	    aarch64_set_vec_u64 (vd, i, aarch64_get_vec_u32 (vn, i + bias)
				 - aarch64_get_vec_u32 (vm, i + bias));
	  break;

	default:
	  return_UNALLOC;
	}
      break;
    }

  return status;
}

#define DO_ADDP(FN)							\
  do									\
    {									\
      for (i = 0; i < range; i++)					\
	{								\
	  aarch64_set_vec_##FN (vd, i,					\
				aarch64_get_vec_##FN (vn, i * 2)	\
				+ aarch64_get_vec_##FN (vn, i * 2 + 1));\
	  aarch64_set_vec_##FN (vd, i + range,				\
				aarch64_get_vec_##FN (vm, i * 2)	\
				+ aarch64_get_vec_##FN (vm, i * 2 + 1));\
	}								\
      }									\
    while (0)
  
static StatusCode
do_vec_ADDP (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,24] = 00 1110
  // instr[23,22] = size: bytes (00), half (01), word (10), long (11)
  // instr[21]    = 1
  // insrt[20,16] = Vm
  // instr[15,10] = 1011 11
  // instr[9,5]   = Vn
  // instr[4,0]   = V dest

  NYI_assert (29, 24, 0x0E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x2F);

  unsigned full = uimm (instr, 30, 30);
  unsigned size = uimm (instr, 23, 22);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i, range;

  switch (size)
    {
    case 0:
      range = full ? 8 : 4;
      DO_ADDP (u8);
      return status;

    case 1:
      range = full ? 4 : 2;
      DO_ADDP (u16);
      return status;

    case 2:
      range = full ? 2 : 1;
      DO_ADDP (u32);
      return status;
      
    case 3:
      if (! full)
	return_UNALLOC;
      range = 1;
      DO_ADDP (u64);
      return status;

    default:
      return_NYI;
    }
}

static StatusCode
do_vec_UMOV (void)
{
  // instr[31]    = 0
  // instr[30]    = 32-bit(0)/64-bit(1)
  // instr[29,21] = 00 1110 000
  // insrt[20,16] = size & index
  // instr[15,10] = 0011 11
  // instr[9,5]   = V source
  // instr[4,0]   = R dest

  NYI_assert (29, 21, 0x070);
  NYI_assert (15, 10, 0x0F);

  unsigned vs = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  unsigned index;

  if (uimm (instr, 16, 16))
    {
      // Byte transfer
      index = uimm (instr, 20, 17);
      aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u8 (vs, index));
    }
  else if (uimm (instr, 17, 17))
    {
      index = uimm (instr, 20, 18);
      aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u16 (vs, index));
    }
  else if (uimm (instr, 18, 18))
    {
      index = uimm (instr, 20, 19);
      aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u32 (vs, index));
    }
  else
    {
      if (uimm (instr, 30, 30) != 1)
	return_UNALLOC;

      index = uimm (instr, 20, 20);
      aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u64 (vs, index));
    }

  return status;
}

static StatusCode
do_vec_FABS (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,23] = 00 1110 1
  // instr[22]    = float(0)/double(1)
  // instr[21,16] = 10 0000
  // instr[15,10] = 1111 10
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  NYI_assert (29, 23, 0x1D);
  NYI_assert (21, 10, 0x83E);

  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  unsigned i;

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_NYI;

      for (i = 0; i < 2; i++)
	aarch64_set_vec_double (vd, i, fabs (aarch64_get_vec_double (vn, i)));
    }
  else
    {
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_float (vd, i, fabsf (aarch64_get_vec_float (vn, i)));
    }

  return status;
}

static StatusCode
do_vec_FCVTZS (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0) / all (1)
  // instr[29,23] = 00 1110 1
  // instr[22]    = single (0) / double (1)
  // instr[21,10] = 10 0001 1011 10
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd

  NYI_assert (31, 31, 0);
  NYI_assert (29, 23, 0x1D);
  NYI_assert (21, 10, 0x86E);

  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  unsigned i;

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_UNALLOC;

      for (i = 0; i < 2; i++)
	aarch64_set_vec_s64 (rd, i,
			     (int64_t) aarch64_get_vec_double (rn, i));
    }
  else
    for (i = 0; i < (full ? 4 : 2); i++)
      aarch64_set_vec_s32 (rd, i,
			   (int32_t) aarch64_get_vec_float (rn, i));
  
  return status;
}

static StatusCode
do_vec_op1 (void)
{
  // instr[31]    = 0
  // instr[30]    = half/full
  // instr[29,24] = 00 1110
  // instr[23,21] = ???
  // instr[20,16] = Vm
  // instr[15,10] = sub-opcode
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x0E);

  if (uimm (instr, 21, 21) == 0)
    {
      if (uimm (instr, 23, 22) == 0)
	{
	  if (uimm (instr, 30, 30) == 1
	      && uimm (instr, 17, 14) == 0
	      && uimm (instr, 12, 10) == 7)
	    return do_vec_ins_2 ();

	  switch (uimm (instr, 15, 10))
	    {
	    case 0x01: return do_vec_DUP_vector_into_vector ();
	    case 0x03: return do_vec_DUP_scalar_into_vector ();
	    case 0x07: return do_vec_INS ();
	    case 0x0A: return do_vec_TRN ();

	    case 0x0F:
	      if (uimm (instr, 17, 16) == 0)
		return do_vec_MOV_into_scalar ();
	      break;

	    case 0x00:
	    case 0x08:
	    case 0x10:
	    case 0x18:
	      return do_vec_TBL ();

	    case 0x06:
	    case 0x16:
	      return do_vec_UZP ();

	    case 0x0E:
	    case 0x1E:
	      return do_vec_ZIP ();
	      
	    default:
	      return_NYI;
	    }
	}

      switch (uimm (instr, 13, 10))
	{
	case 0x6: return do_vec_UZP ();
	case 0xE: return do_vec_ZIP ();
	case 0xA: return do_vec_TRN ();
	case 0xF: return do_vec_UMOV ();
	default:  return_NYI;
	}
    }

  switch (uimm (instr, 15, 10))
    {
    case 0x07:
      switch (uimm (instr, 23, 21))
	{
	case 1: return do_vec_AND ();
	case 5: return do_vec_ORR ();
	case 7: return do_vec_ORN ();
	default: return_NYI;
	}

    case 0x08: return do_vec_sub_long ();
    case 0x0a: return do_vec_XTN ();
    case 0x11: return do_vec_SSHL ();
    case 0x19: return do_vec_max ();
    case 0x1B: return do_vec_min ();
    case 0x21: return do_vec_add ();
    case 0x25: return do_vec_MLA ();
    case 0x27: return do_vec_mul ();
    case 0x2F: return do_vec_ADDP ();
    case 0x30: return do_vec_mull ();
    case 0x33: return do_vec_FMLA ();
    case 0x35: return do_vec_fadd ();

    case 0x2E:
      switch (uimm (instr, 20, 16))
	{
	case 0x00: return do_vec_ABS ();
	case 0x01: return do_vec_FCVTZS ();
	case 0x11: return do_vec_ADDV ();
	  
	default: return_NYI;
	}
      
    case 0x31:
    case 0x3B:
      return do_vec_Fminmax ();
      
    case 0x0D:
    case 0x0F:
    case 0x22:
    case 0x23:
    case 0x26:
    case 0x2A:
    case 0x32:
    case 0x36:
    case 0x39:
    case 0x3A:
      return do_vec_compare ();

    case 0x3E:
      return do_vec_FABS ();
    default:
      return_NYI;
    }
}

static StatusCode
do_vec_xtl (void)
{
  // instr[31]    = 0
  // instr[30,29] = SXTL (00), UXTL (01), SXTL2 (10), UXTL2 (11)
  // instr[28,22] = 0 1111 00
  // instr[21,16] = size & shift (USHLL, SSHLL, USHLL2, SSHLL2)
  // instr[15,10] = 1010 01
  // instr[9,5]   = V source
  // instr[4,0]   = V dest

  NYI_assert (28, 22, 0x3C);
  NYI_assert (15, 10, 0x29);

  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i, shift, bias = 0;

  switch (uimm (instr, 30, 29))
    {
    case 2: // SXTL2, SSHLL2
      bias = 2;
    case 0: // SXTL, SSHLL
      if (uimm (instr, 21, 21))
	{
	  shift = uimm (instr, 20, 16);
	  aarch64_set_vec_s64 (vd, 0, aarch64_get_vec_s32 (vs, bias) << shift);
	  aarch64_set_vec_s64 (vd, 1, aarch64_get_vec_s32 (vs, bias + 1) << shift);
	}
      else if (uimm (instr, 20, 20))
	{
	  shift = uimm (instr, 19, 16);
	  bias *= 2;
	  for (i = 0; i < 4; i++)
	    aarch64_set_vec_s32 (vd, i, aarch64_get_vec_s16 (vs, i + bias) << shift);
	}
      else 
	{
	  NYI_assert (19, 19, 1);

	  shift = uimm (instr, 18, 16);
	  bias *= 3;
	  for (i = 0; i < 8; i++)
	    aarch64_set_vec_s16 (vd, i, aarch64_get_vec_s8 (vs, i + bias) << shift);
	}
      return status;

    case 3: // UXTL2, USHLL2
      bias = 2;
    case 1: // UXTL, USHLL
      if (uimm (instr, 21, 21))
	{
	  shift = uimm (instr, 20, 16);
	  aarch64_set_vec_u64 (vd, 0, aarch64_get_vec_u32 (vs, bias) << shift);
	  aarch64_set_vec_u64 (vd, 1, aarch64_get_vec_u32 (vs, bias + 1) << shift);
	}
      else if (uimm (instr, 20, 20))
	{
	  shift = uimm (instr, 19, 16);
	  bias *= 2;
	  for (i = 0; i < 4; i++)
	    aarch64_set_vec_u32 (vd, i, aarch64_get_vec_u16 (vs, i + bias) << shift);
	}
      else 
	{
	  NYI_assert (19, 19, 1);

	  shift = uimm (instr, 18, 16);
	  bias *= 3;
	  for (i = 0; i < 8; i++)
	    aarch64_set_vec_u16 (vd, i, aarch64_get_vec_u8 (vs, i + bias) << shift);
	}
      return status;

    default:
      return_NYI;
    }
}

static StatusCode
do_vec_SHL (void)
{
  // instr [31]    = 0
  // instr [30]    = half(0)/full(1)
  // instr [29,23] = 001 1110
  // instr [22,16] = size and shift amount
  // instr [15,10] = 01 0101
  // instr [9, 5]  = Vs
  // instr [4, 0]  = Vd

  NYI_assert (29, 23, 0x1E);
  NYI_assert (15, 10, 0x15);

  int full = uimm (instr, 30, 30);
  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  if (uimm (instr, 22, 22))
    {
      if (full == 0)
	return_UNALLOC;

      int shift = uimm (instr, 21, 16) - 1;
      for (i = 0; i < 2; i++)
	{
	  u_int64_t val = aarch64_get_vec_u64 (vs, i);
	  aarch64_set_vec_u64 (vd, i, val << shift);
	}
	
      return status;
    }

  if (uimm (instr, 21, 21))
    {
      int shift = uimm (instr, 20, 16) - 1;

      for (i = 0; i < (full ? 4 : 2); i++)
	{
	  u_int32_t val = aarch64_get_vec_u32 (vs, i);
	  aarch64_set_vec_u32 (vd, i, val << shift);
	}
	
      return status;
    }

  if (uimm (instr, 20, 20))
    {
      int shift = uimm (instr, 19, 16) - 1;

      for (i = 0; i < (full ? 8 : 4); i++)
	{
	  u_int16_t val = aarch64_get_vec_u16 (vs, i);
	  aarch64_set_vec_u16 (vd, i, val << shift);
	}
	
      return status;
    }

  if (uimm (instr, 19, 19) == 0)
    return_UNALLOC;

  int shift = uimm (instr, 18, 16) - 1;

  for (i = 0; i < (full ? 16 : 8); i++)
    {
      u_int8_t val = aarch64_get_vec_u8 (vs, i);
      aarch64_set_vec_u8 (vd, i, val << shift);
    }

  return status;
}

static StatusCode
do_vec_SSHR_USHR (void)
{
  // instr [31]    = 0
  // instr [30]    = half(0)/full(1)
  // instr [29]    = signed(0)/unsigned(1)
  // instr [28,23] = 01 1110
  // instr [22,16] = size and shift amount
  // instr [15,10] = 0000 01 
  // instr [9, 5]  = Vs
  // instr [4, 0]  = Vd

  NYI_assert (28, 23, 0x1E);
  NYI_assert (15, 10, 0x01);

  int full = uimm (instr, 30, 30);
  int sign = uimm (instr, 29, 29);
  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  if (uimm (instr, 22, 22))
    {
      if (full == 0)
	return_UNALLOC;

      int shift = uimm (instr, 21, 16);
      if (sign)
	for (i = 0; i < 2; i++)
	  {
	    int64_t val = aarch64_get_vec_s64 (vs, i);
	    aarch64_set_vec_s64 (vd, i, val >> shift);
	  }
      else
	for (i = 0; i < 2; i++)
	  {
	    u_int64_t val = aarch64_get_vec_u64 (vs, i);
	    aarch64_set_vec_u64 (vd, i, val >> shift);
	  }
	
      return status;
    }

  if (uimm (instr, 21, 21))
    {
      int shift = uimm (instr, 20, 16);

      if (sign)
	for (i = 0; i < (full ? 4 : 2); i++)
	  {
	    int32_t val = aarch64_get_vec_s32 (vs, i);
	    aarch64_set_vec_s32 (vd, i, val >> shift);
	  }
      else
	for (i = 0; i < (full ? 4 : 2); i++)
	  {
	    u_int32_t val = aarch64_get_vec_u32 (vs, i);
	    aarch64_set_vec_u32 (vd, i, val >> shift);
	  }
	
      return status;
    }

  if (uimm (instr, 20, 20))
    {
      int shift = uimm (instr, 19, 16);

      if (sign)
	for (i = 0; i < (full ? 8 : 4); i++)
	  {
	    int16_t val = aarch64_get_vec_s16 (vs, i);
	    aarch64_set_vec_s16 (vd, i, val >> shift);
	  }
      else
	for (i = 0; i < (full ? 8 : 4); i++)
	  {
	    u_int16_t val = aarch64_get_vec_u16 (vs, i);
	    aarch64_set_vec_u16 (vd, i, val >> shift);
	  }
	
      return status;
    }

  if (uimm (instr, 19, 19) == 0)
    return_UNALLOC;

  int shift = uimm (instr, 18, 16);

  if (sign)
    for (i = 0; i < (full ? 16 : 8); i++)
      {
	int8_t val = aarch64_get_vec_s8 (vs, i);
	aarch64_set_vec_s8 (vd, i, val >> shift);
      }
  else
    for (i = 0; i < (full ? 16 : 8); i++)
      {
	u_int8_t val = aarch64_get_vec_u8 (vs, i);
	aarch64_set_vec_u8 (vd, i, val >> shift);
      }

  return status;
}

static StatusCode
do_vec_op2 (void)
{
  // instr[31]    = 0
  // instr[30]    = half/full
  // instr[29,24] = 00 1111
  // instr[23]    = ?
  // instr[22,16] = element size & index
  // instr[15,10] = sub-opcode
  // instr[9,5]   = Vm
  // instr[4.0]   = Vd

  NYI_assert (29, 24, 0x0F);
  
  if (uimm (instr, 23, 23) != 0)
    return_NYI;

  switch (uimm (instr, 15, 10))
    {
    case 0x01: return do_vec_SSHR_USHR ();
    case 0x15: return do_vec_SHL ();
    case 0x29: return do_vec_xtl ();
    default:   return_NYI;
    }
}

static StatusCode
do_vec_neg (void)
{
  // instr[31]    = 0
  // instr[30]    = full(1)/half(0)
  // instr[29,24] = 10 1110
  // instr[23,22] = size: byte(00), half (01), word (10), long (11)
  // instr[21,10] = 1000 0010 1110
  // instr[9,5]   = Vs
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x2E);
  NYI_assert (21, 10, 0x82E);
  
  int    full = uimm (instr, 30, 30);
  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_s8 (vd, i, - aarch64_get_vec_s8 (vs, i));
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_s16 (vd, i, - aarch64_get_vec_s16 (vs, i));
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_s32 (vd, i, - aarch64_get_vec_s32 (vs, i));
      return status;

    case 3:
      if (! full)
	return_NYI;
      for (i = 0; i < 2; i++)
	aarch64_set_vec_s64 (vd, i, - aarch64_get_vec_s64 (vs, i));
      return status;

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

static StatusCode
do_vec_sqrt (void)
{
  // instr[31]    = 0
  // instr[30]    = full(1)/half(0)
  // instr[29,23] = 101 1101
  // instr[22]    = single(0)/double(1)
  // instr[21,10] = 1000 0111 1110
  // instr[9,5]   = Vs
  // instr[4,0]   = Vd

  NYI_assert (29, 23, 0x5B);
  NYI_assert (21, 10, 0x87E);
  
  int    full = uimm (instr, 30, 30);
  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  if (uimm (instr, 22, 22) == 0)
    for (i = 0; i < (full ? 4 : 2); i++)
      aarch64_set_vec_float (vd, i, sqrtf (aarch64_get_vec_float (vs, i)));
  else
    for (i = 0; i < 2; i++)
      aarch64_set_vec_double (vd, i, sqrt (aarch64_get_vec_double (vs, i)));

  return status;
}

static StatusCode
do_vec_mls_indexed (void)
{
  // instr[31]       = 0
  // instr[30]       = half(0)/full(1)
  // instr[29,24]    = 10 1111
  // instr[23,22]    = 16-bit(01)/32-bit(10)
  // instr[21,20+11] = index (if 16-bit)
  // instr[21+11]    = index (if 32-bit)  
  // instr[20,16]    = Vm
  // instr[15,12]    = 0100
  // instr[11]       = part of index
  // instr[10]       = 0
  // instr[9,5]      = Vs
  // instr[4,0]      = Vd

  NYI_assert (15, 12, 4);
  NYI_assert (10, 10, 0);

  int    full = uimm (instr, 30, 30);
  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned vm = uimm (instr, 20, 16);
  unsigned i;

  switch (uimm (instr, 23, 22))
    {
    case 1:
      if (vm > 15)
	return_NYI;
      unsigned elem = (uimm (instr, 21, 20) << 1) | uimm (instr, 11, 11);
      u_int32_t val = aarch64_get_vec_u16 (vm, elem);
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_u32 (vd, i,
			     aarch64_get_vec_u32 (vd, i) -
			     (aarch64_get_vec_u32 (vs, i) * val));
      return status;

    case 2:
      {
	unsigned elem = (uimm (instr, 21, 21) << 1) | uimm (instr, 11, 11);
	u_int64_t val = aarch64_get_vec_u32 (vm, elem);

	for (i = 0; i < (full ? 4 : 2); i++)
	  aarch64_set_vec_u64 (vd, i,
			       aarch64_get_vec_u64 (vd, i) -
			       (aarch64_get_vec_u64 (vs, i) * val));
	return status;
      }
      
    case 0:
    case 3:
    default:
      return_NYI;
    }
}

static StatusCode
do_vec_SUB (void)
{
  // instr [31]    = 0
  // instr [30]    = half(0)/full(1)
  // instr [29,24] = 10 1110
  // instr [23,22] = size: byte(00, half(01), word (10), long (11)
  // instr [21]    = 1
  // instr [20,16] = Vm
  // instr [15,10] = 10 0001
  // instr [9, 5]  = Vn
  // instr [4, 0]  = Vd

  NYI_assert (29, 24, 0x2E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x21);
  
  unsigned full = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_s8 (vd, i,
			    aarch64_get_vec_s8 (vn, i)
			    - aarch64_get_vec_s8 (vm, i));
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_s16 (vd, i,
			     aarch64_get_vec_s16 (vn, i)
			     - aarch64_get_vec_s16 (vm, i));
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_s32 (vd, i,
			     aarch64_get_vec_s32 (vn, i)
			     - aarch64_get_vec_s32 (vm, i));
      return status;

    case 3:
      if (full == 0)
	return_UNALLOC;

      for (i = 0; i < 2; i++)
	aarch64_set_vec_s64 (vd, i,
			     aarch64_get_vec_s64 (vn, i)
			     - aarch64_get_vec_s64 (vm, i));
      return status;

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

static StatusCode
do_vec_MLS (void)
{
  // instr [31]    = 0
  // instr [30]    = half(0)/full(1)
  // instr [29,24] = 10 1110
  // instr [23,22] = size: byte(00, half(01), word (10)
  // instr [21]    = 1
  // instr [20,16] = Vm
  // instr [15,10] = 10 0101
  // instr [9, 5]  = Vn
  // instr [4, 0]  = Vd

  NYI_assert (29, 24, 0x2E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x25);

  unsigned full = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  switch (uimm (instr, 23, 22))
    {
    case 0:
      for (i = 0; i < (full ? 16 : 8); i++)
	aarch64_set_vec_u8 (vd, i,
			    (aarch64_get_vec_u8 (vn, i)
			     * aarch64_get_vec_u8 (vm, i))
			    - aarch64_get_vec_u8 (vd, i));
      return status;

    case 1:
      for (i = 0; i < (full ? 8 : 4); i++)
	aarch64_set_vec_u16 (vd, i,
			     (aarch64_get_vec_u16 (vn, i)
			      * aarch64_get_vec_u16 (vm, i))
			     - aarch64_get_vec_u16 (vd, i));
      return status;

    case 2:
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i,
			     (aarch64_get_vec_u32 (vn, i)
			      * aarch64_get_vec_u32 (vm, i))
			     - aarch64_get_vec_u32 (vd, i));
      return status;
      
    default:
      return_UNALLOC;
    }
}

static StatusCode
do_vec_FDIV (void)
{
  // instr [31]    = 0
  // instr [30]    = half(0)/full(1)
  // instr [29,23] = 10 1110 0
  // instr [22]    = float()/double(1)
  // instr [21]    = 1
  // instr [20,16] = Vm
  // instr [15,10] = 1111 11
  // instr [9, 5]  = Vn
  // instr [4, 0]  = Vd

  NYI_assert (29, 23, 0x5C);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x3F);

  unsigned full = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_UNALLOC;

      for (i = 0; i < 2; i++)
	aarch64_set_vec_double (vd, i,
				aarch64_get_vec_double (vn, i)
				/ aarch64_get_vec_double (vm, i));
      return status;
    }

  for (i = 0; i < (full ? 4 : 2); i++)
    aarch64_set_vec_float (vd, i,
			   aarch64_get_vec_float (vn, i)
			   / aarch64_get_vec_float (vm, i));
  return status;
}

static StatusCode
do_vec_FMUL (void)
{
  // instr [31]    = 0
  // instr [30]    = half(0)/full(1)
  // instr [29,23] = 10 1110 0
  // instr [22]    = float(0)/double(1)
  // instr [21]    = 1
  // instr [20,16] = Vm
  // instr [15,10] = 1101 11
  // instr [9, 5]  = Vn
  // instr [4, 0]  = Vd

  NYI_assert (29, 23, 0x5C);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x37);

  unsigned full = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_UNALLOC;

      for (i = 0; i < 2; i++)
	aarch64_set_vec_double (vd, i,
				aarch64_get_vec_double (vn, i)
				* aarch64_get_vec_double (vm, i));
      return status;
    }

  for (i = 0; i < (full ? 4 : 2); i++)
    aarch64_set_vec_float (vd, i,
			   aarch64_get_vec_float (vn, i)
			   * aarch64_get_vec_float (vm, i));
  return status;
}

static StatusCode
do_vec_FADDP (void)
{
  // instr [31]    = 0
  // instr [30]    = half(0)/full(1)
  // instr [29,23] = 10 1110 0
  // instr [22]    = float(0)/double(1)
  // instr [21]    = 1
  // instr [20,16] = Vm
  // instr [15,10] = 1101 01
  // instr [9, 5]  = Vn
  // instr [4, 0]  = Vd

  NYI_assert (29, 23, 0x5C);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x35);

  unsigned full = uimm (instr, 30, 30);
  unsigned vm = uimm (instr, 20, 16);
  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_UNALLOC;

      aarch64_set_vec_double (vd, 0, aarch64_get_vec_double (vn, 0)
			      + aarch64_get_vec_double (vn, 1));
      aarch64_set_vec_double (vd, 1, aarch64_get_vec_double (vm, 0)
			      + aarch64_get_vec_double (vm, 1));
    }
  else
    {
      aarch64_set_vec_float (vd, 0, aarch64_get_vec_float (vn, 0)
			     + aarch64_get_vec_float (vn, 1));
      if (full)
	aarch64_set_vec_float (vd, 1, aarch64_get_vec_float (vn, 2)
			       + aarch64_get_vec_float (vn, 3));
      aarch64_set_vec_float (vd, full ? 2 : 1,
			     aarch64_get_vec_float (vm, 0)
			     + aarch64_get_vec_float (vm, 1));
      if (full)
	aarch64_set_vec_float (vd, 3,
			       aarch64_get_vec_float (vm, 2)
			       + aarch64_get_vec_float (vm, 3));
    }

  return status;
}

static StatusCode
do_vec_FSQRT (void)
{
  // instr[31]    = 0
  // instr[30]    = half(0)/full(1)
  // instr[29,23] = 10 1110 1
  // instr[22]    = single(0)/double(1)
  // instr[21,10] = 10 0001 1111 10
  // instr[9,5]   = Vsrc
  // instr[4,0]   = Vdest

  NYI_assert (29, 23, 0x5D);
  NYI_assert (21, 10, 0x87E);

  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  int i;

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_UNALLOC;

      for (i = 0; i < 2; i++)
	aarch64_set_vec_double (vd, i, sqrt (aarch64_get_vec_double (vn, i)));
    }
  else
    {
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_float (vd, i, sqrtf (aarch64_get_vec_float (vn, i)));
    }

  return status;
}

static StatusCode
do_vec_FNEG (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,23] = 10 1110 1
  // instr[22]    = single (0)/double (1)
  // instr[21,10] = 10 0000 1111 10
  // instr[9,5]   = Vsrc
  // instr[4,0]   = Vdest

  NYI_assert (29, 23, 0x5D);
  NYI_assert (21, 10, 0x83E);

  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned full = uimm (instr, 30, 30);
  int i;

  if (uimm (instr, 22, 22))
    {
      if (! full)
	return_UNALLOC;

      for (i = 0; i < 2; i++)
	aarch64_set_vec_double (vd, i, - aarch64_get_vec_double (vn, i));
    }
  else
    {
      for (i = 0; i < (full ? 4 : 2); i++)
	aarch64_set_vec_float (vd, i, - aarch64_get_vec_float (vn, i));
    }

  return status;
}

static StatusCode
do_vec_NOT (void)
{
  // instr[31]    = 0
  // instr[30]    = half (0)/full (1)
  // instr[29,21] = 10 1110 001
  // instr[20,16] = 0 0000
  // instr[15,10] = 0101 10
  // instr[9,5]   = Vn
  // instr[4.0]   = Vd

  NYI_assert (29, 10, 0xB8816);

  unsigned vn = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned i;
  int      full = uimm (instr, 30, 30);

  for (i = 0; i < (full ? 16 : 8); i++)
    aarch64_set_vec_u8 (vd, i, ~ aarch64_get_vec_u8 (vn, i));

  return status;
}

static StatusCode
do_vec_MOV_element (void)
{
  // instr[31,21] = 0110 1110 000
  // instr[20,16] = size & dest index
  // instr[15]    = 0
  // instr[14,11] = source index
  // instr[10]    = 1
  // instr[9,5]   = Vs
  // instr[4.0]   = Vd

  NYI_assert (31, 21, 0x370);
  NYI_assert (15, 15, 0);
  NYI_assert (10, 10, 1);

  unsigned vs = uimm (instr, 9, 5);
  unsigned vd = uimm (instr, 4, 0);
  unsigned src_index;
  unsigned dst_index;

  if (uimm (instr, 16, 16))
    {
      // Move a byte.
      src_index = uimm (instr, 14, 11);
      dst_index = uimm (instr, 20, 17);
      aarch64_set_vec_u8 (vd, dst_index, aarch64_get_vec_u8 (vs, src_index));
    }
  else if (uimm (instr, 17, 17))
    {
      // Move 16-bits.
      NYI_assert (11, 11, 0);
      src_index = uimm (instr, 14, 12);
      dst_index = uimm (instr, 20, 18);
      aarch64_set_vec_u16 (vd, dst_index, aarch64_get_vec_u16 (vs, src_index));
    }
  else if (uimm (instr, 18, 18))
    {
      // Move 32-bits.
      NYI_assert (12, 11, 0);
      src_index = uimm (instr, 14, 13);
      dst_index = uimm (instr, 20, 19);
      aarch64_set_vec_u32 (vd, dst_index, aarch64_get_vec_u32 (vs, src_index));
    }
  else
    {
      NYI_assert (19, 19, 1);
      NYI_assert (13, 11, 0);
      src_index = uimm (instr, 14, 14);
      dst_index = uimm (instr, 20, 20);
      aarch64_set_vec_u64 (vd, dst_index, aarch64_get_vec_u64 (vs, src_index));
    }

  return status;
}

static StatusCode
dexAdvSIMD0 (void)
{
  // instr [28,25] = 0 111
  if (    uimm (instr, 15, 10) == 0x07
      && (uimm (instr, 9, 5) == uimm (instr, 20, 16)))
    {
      if (uimm (instr, 31, 21) == 0x075
	  || uimm (instr, 31, 21) == 0x275)
	return do_vec_MOV_whole_vector ();
    }

  if (uimm (instr, 29, 19) == 0x1E0)
    return do_vec_MOV_immediate ();

  if (uimm (instr, 29, 19) == 0x5E0)
    return do_vec_MVNI ();

  if (uimm (instr, 29, 19) == 0x1C0 || uimm (instr, 29, 19) == 0x1C1)
    {
      if (uimm (instr, 15, 10) == 0x03)
	return do_vec_DUP_scalar_into_vector ();
    }

  switch (uimm (instr, 29, 24))
    {
    case 0x0E: return do_vec_op1 ();
    case 0x0F: return do_vec_op2 ();

    case 0x2f:
      switch (uimm (instr, 15, 10))
	{
	case 0x01: return do_vec_SSHR_USHR ();
	case 0x10:
	case 0x12: return do_vec_mls_indexed ();
	case 0x29: return do_vec_xtl ();
	default:
	  return_NYI;
	}

    case 0x2E:
      if (uimm (instr, 21, 21) == 1)
	{
	  switch (uimm (instr, 15, 10))
	    {
	    case 0x07:
	      switch (uimm (instr, 23, 22))
		{
		case 0: return do_vec_EOR ();
		case 1: return do_vec_BSL ();
		case 2: 
		case 3: return do_vec_bit ();
		}
	      break;

	    case 0x08: return do_vec_sub_long ();
	    case 0x11: return do_vec_USHL ();
	    case 0x16: return do_vec_NOT ();
	    case 0x19: return do_vec_max ();
	    case 0x1B: return do_vec_min ();
	    case 0x21: return do_vec_SUB ();
	    case 0x25: return do_vec_MLS ();
	    case 0x31: return do_vec_FminmaxNMP ();
	    case 0x35: return do_vec_FADDP ();
	    case 0x37: return do_vec_FMUL ();
	    case 0x3F: return do_vec_FDIV ();

	    case 0x3E:
	      switch (uimm (instr, 20, 16))
		{
		case 0x00: return do_vec_FNEG ();
		case 0x01: return do_vec_FSQRT ();
		default:   return_NYI;
		}

	    case 0x0D:
	    case 0x0F:
	    case 0x22:
	    case 0x23:
	    case 0x26:
	    case 0x2A:
	    case 0x32:
	    case 0x36:
	    case 0x39:
	    case 0x3A:
	      return do_vec_compare ();
	      
	    default: break;
	    }
	}

      if (uimm (instr, 31, 21) == 0x370)
	return do_vec_MOV_element ();
      
      switch (uimm (instr, 21, 10))
	{
	case 0x82E: return do_vec_neg ();
	case 0x87E: return do_vec_sqrt ();
	default:
	  if (uimm (instr, 15, 10) == 0x30)
	    return do_vec_mull ();
	  break;
	}
      break;

    default:
      break;
    }

  return_NYI;
}

// 3 sources

// float multiply add
static StatusCode
fmadds (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd, aarch64_get_FP_float (sa)
			       + aarch64_get_FP_float (sn)
			       * aarch64_get_FP_float (sm));
}

// double multiply add
static StatusCode
fmaddd (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd, aarch64_get_FP_double (sa)
				+ aarch64_get_FP_double (sn)
				* aarch64_get_FP_double (sm));
}

// float multiply subtract
static StatusCode
fmsubs (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd, aarch64_get_FP_float (sa)
			       - aarch64_get_FP_float (sn)
			       * aarch64_get_FP_float (sm));
}

// double multiply subtract
static StatusCode
fmsubd (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd, aarch64_get_FP_double (sa)
				- aarch64_get_FP_double (sn)
				* aarch64_get_FP_double (sm));
}

// float negative multiply add
static StatusCode
fnmadds (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd, - aarch64_get_FP_float (sa)
			       + (- aarch64_get_FP_float (sn))
			       * aarch64_get_FP_float (sm));
}

// double negative multiply add
static StatusCode
fnmaddd (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd, - aarch64_get_FP_double (sa)
				+ (- aarch64_get_FP_double (sn))
				* aarch64_get_FP_double (sm));
}

// float negative multiply subtract
static StatusCode
fnmsubs (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd, - aarch64_get_FP_float (sa)
			       + aarch64_get_FP_float (sn)
			       * aarch64_get_FP_float (sm));
}

// double negative multiply subtract
static StatusCode
fnmsubd (void)
{
  unsigned sa = uimm (instr, 14, 10);
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd, - aarch64_get_FP_double (sa)
				+ aarch64_get_FP_double (sn)
				* aarch64_get_FP_double (sm));
}

static StatusCode
dexSimpleFPDataProc3Source (void)
{
  // instr[31]    ==> M : 0 ==> OK, 1 ==> UNALLOC
  // instr[30]    = 0
  // instr[29]    ==> S :  0 ==> OK, 1 ==> UNALLOC
  // instr[28,25] = 1111 
  // instr[24]    = 1
  // instr[23,22] ==> type : 0 ==> single, 01 ==> double, 1x ==> UNALLOC
  // instr[21]    ==> o1 : 0 ==> unnegated, 1 ==> negated
  // instr[15]    ==> o2 : 0 ==> ADD, 1 ==> SUB

  u_int32_t M_S = (uimm (instr, 31, 31) << 1) | uimm (instr, 29, 29);
  if (M_S != 0)
    return_UNALLOC;

  // dispatch on combined type:o1:o2
  u_int32_t dispatch = (uimm (instr, 23, 21) << 1) | uimm (instr, 15, 15);

  switch (dispatch)
    {
    case 0: return fmadds ();
    case 1: return fmsubs ();
    case 2: return fnmadds ();
    case 3: return fnmsubs ();
    case 4: return fmaddd ();
    case 5: return fmsubd ();
    case 6: return fnmaddd ();
    case 7: return fnmsubd ();
    default:
      // type > 1 is currently unallocated
      return_UNALLOC;
    }
}

static StatusCode
dexSimpleFPFixedConvert (void)
{
  return_NYI;
}

static StatusCode
dexSimpleFPCondCompare (void)
{
  return_NYI;
}

// 2 sources

// float add
static StatusCode
fadds (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd, aarch64_get_FP_float (sn) + aarch64_get_FP_float (sm));
}

// double add
static StatusCode
faddd (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd, aarch64_get_FP_double (sn) + aarch64_get_FP_double (sm));
}

// float divide
static StatusCode
fdivs (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd, aarch64_get_FP_float (sn) / aarch64_get_FP_float (sm));
}

// double divide
static StatusCode
fdivd (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd, aarch64_get_FP_double (sn) / aarch64_get_FP_double (sm));
}

// float multiply
static StatusCode
fmuls (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd,aarch64_get_FP_float (sn) * aarch64_get_FP_float (sm));
}

// double multiply
static StatusCode
fmuld (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd,aarch64_get_FP_double (sn) * aarch64_get_FP_double (sm));
}

// float negate and multiply
static StatusCode
fnmuls (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd,- (aarch64_get_FP_float (sn) * aarch64_get_FP_float (sm)));
}

// double negate and multiply
static StatusCode
fnmuld (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd,- (aarch64_get_FP_double (sn) * aarch64_get_FP_double (sm)));
}

// float subtract
static StatusCode
fsubs (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_float (sd,aarch64_get_FP_float (sn) - aarch64_get_FP_float (sm));
}

// double subtract
static StatusCode
fsubd (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  return aarch64_set_FP_double (sd,aarch64_get_FP_double (sn) - aarch64_get_FP_double (sm));
}

static StatusCode
do_FMINNM (void)
{
  // instr[31,23] = 0 0011 1100
  // instr[22]    = float(0)/double(1)
  // instr[21]    = 1
  // instr[20,16] = Sm
  // instr[15,10] = 01 1110
  // instr[9,5]   = Sn
  // instr[4,0]   = Sd

  NYI_assert (31, 23, 0x03C);
  NYI_assert (15, 10, 0x1E);

  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  if (uimm (instr, 22, 22))
    return aarch64_set_FP_double (sd, dminnm (aarch64_get_FP_double (sn),
					       aarch64_get_FP_double (sm)));

  return aarch64_set_FP_float (sd, fminnm (aarch64_get_FP_float (sn),
					    aarch64_get_FP_float (sm)));
}

static StatusCode
do_FMAXNM (void)
{
  // instr[31,23] = 0 0011 1100
  // instr[22]    = float(0)/double(1)
  // instr[21]    = 1
  // instr[20,16] = Sm
  // instr[15,10] = 01 1010
  // instr[9,5]   = Sn
  // instr[4,0]   = Sd

  NYI_assert (31, 23, 0x03C);
  NYI_assert (15, 10, 0x1A);

  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);
  unsigned sd = uimm (instr,  4,  0);

  if (uimm (instr, 22, 22))
    return aarch64_set_FP_double (sd, dmaxnm (aarch64_get_FP_double (sn),
					       aarch64_get_FP_double (sm)));

  return aarch64_set_FP_float (sd, fmaxnm (aarch64_get_FP_float (sn),
					    aarch64_get_FP_float (sm)));
}

static StatusCode
dexSimpleFPDataProc2Source (void)
{
  // instr[31]    ==> M : 0 ==> OK, 1 ==> UNALLOC
  // instr[30]    = 0
  // instr[29]    ==> S :  0 ==> OK, 1 ==> UNALLOC
  // instr[28,25] = 1111 
  // instr[24]    = 0
  // instr[23,22] ==> type : 0 ==> single, 01 ==> double, 1x ==> UNALLOC
  // instr[21]    = 1
  // instr[20,16] = Vm
  // instr[15,12] ==> opcode : 0000 ==> FMUL, 0001 ==> FDIV
  //                           0010 ==> FADD, 0011 ==> FSUB,
  //                           0100 ==> FMAX, 0101 ==> FMIN
  //                           0110 ==> FMAXNM, 0111 ==> FMINNM
  //                           1000 ==> FNMUL, ow ==> UNALLOC
  // instr[11,10] = 10
  // instr[9,5]   = Vn
  // instr[4,0]   = Vd

  u_int32_t M_S = (uimm (instr, 31, 31) << 1) | uimm (instr, 29, 29);
  if (M_S != 0)
    return_UNALLOC;

  u_int32_t type = uimm (instr, 23, 22);
  if (type > 1)
    return_UNALLOC;

  // dispatch on opcode
  u_int32_t dispatch = uimm (instr, 15, 12);

  if (type)
    switch (dispatch)
      {
      case 0: return fmuld ();
      case 1: return fdivd ();
      case 2: return faddd ();
      case 3: return fsubd ();
      case 6: return do_FMAXNM ();
      case 7: return do_FMINNM ();
      case 8: return fnmuld ();

	// have not yet implemented fmax and fmin 
      case 4:
      case 5:
	return_NYI;

      default:
	return_UNALLOC;
      }
  else /* type == 0 => floats.  */
    switch (dispatch)
      {
      case 0: return fmuls ();
      case 1: return fdivs ();
      case 2: return fadds ();
      case 3: return fsubs ();
      case 6: return do_FMAXNM ();
      case 7: return do_FMINNM ();
      case 8: return fnmuls ();

      case 4:
      case 5:
	return_NYI;

      default:
	return_UNALLOC;
      }
}

static StatusCode
dexSimpleFPCondSelect (void)
{
  return_NYI;
}

// store 32 bit unscaled signed 9 bit
static StatusCode
fsturs (int32_t offset)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);

  return aarch64_set_mem_float (aarch64_get_reg_u64 (st, 1) + offset,
				aarch64_get_FP_float (rn));
}

// store 64 bit unscaled signed 9 bit
static StatusCode
fsturd (int32_t offset)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);

  return aarch64_set_mem_double (aarch64_get_reg_u64 (st, 1) + offset,
				 aarch64_get_FP_double (rn));
}

// store 128 bit unscaled signed 9 bit
static StatusCode
fsturq (int32_t offset)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);

  FRegister a;
  aarch64_get_FP_long_double (rn, & a);
  return aarch64_set_mem_long_double (aarch64_get_reg_u64 (st, 1) + offset, a);
}

// TODO FP move register

// 32 bit fp to fp move register
static StatusCode
ffmovs (void)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);
  return aarch64_set_FP_float (st, aarch64_get_FP_float (rn));
}

// 64 bit fp to fp move register
static StatusCode
ffmovd (void)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);
  return aarch64_set_FP_double (st, aarch64_get_FP_double (rn));
}

// 32 bit GReg to Vec move register
static StatusCode
fgmovs (void)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);
  return aarch64_set_vec_u32 (st, 0, aarch64_get_reg_u32 (rn, NO_SP));
}

// 64 bit g to fp move register
static StatusCode
fgmovd (void)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);
  return aarch64_set_vec_u64 (st, 0, aarch64_get_reg_u64 (rn, NO_SP));
}

// 32 bit fp to g move register
static StatusCode
gfmovs (void)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (st, NO_SP, aarch64_get_vec_u32 (rn, 0));
}

// 64 bit fp to g move register
static StatusCode
gfmovd (void)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (st, NO_SP, aarch64_get_vec_u64 (rn, 0));
}

// FP move immediate
//
// these install an immediate 8 bit value in the target register
// where the 8 bits comprise 1 sign bit, 4 bits of fraction and a 3
// bit exponent

static StatusCode
fmovs (void)
{
  unsigned int sd = uimm (instr, 4, 0);
  u_int32_t imm = uimm (instr, 20, 13);
  float f = fp_immediate_for_encoding_32 (imm);

  return aarch64_set_FP_float (sd, f);
}

static StatusCode
fmovd (void)
{
  unsigned int sd = uimm (instr, 4, 0);
  u_int32_t imm = uimm (instr, 20, 13);
  double d = fp_immediate_for_encoding_64 (imm);

  return aarch64_set_FP_double (sd, d);
}

static StatusCode
dexSimpleFPImmediate (void)
{
  // instr[31,23] == 00111100 
  // instr[22]    == type : single(0)/double(1)
  // instr[21]    == 1
  // instr[20,13] == imm8
  // instr[12,10] == 100
  // instr[9,5]   == imm5 : 00000 ==> PK, ow ==> UNALLOC
  // instr[4,0]   == Rd

  NYI_assert (31, 23, 0x3C);

  u_int32_t imm5 = uimm (instr, 9, 5);
  if (imm5 != 0)
    return_UNALLOC;

  if (uimm (instr, 22, 22))
    return fmovd ();

  return fmovs ();
}

// TODO specific decode and execute for group Load Store

// TODO FP load/store single register (unscaled offset)

// TODO load 8 bit unscaled signed 9 bit
// TODO load 16 bit unscaled signed 9 bit

// load 32 bit unscaled signed 9 bit
static StatusCode
fldurs (int32_t offset)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);

  return aarch64_set_FP_float (st, aarch64_get_mem_float (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// load 64 bit unscaled signed 9 bit
static StatusCode
fldurd (int32_t offset)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);

  return aarch64_set_FP_double (st, aarch64_get_mem_double (aarch64_get_reg_u64 (rn, SP_OK) + offset));
}

// load 128 bit unscaled signed 9 bit
static StatusCode
fldurq (int32_t offset)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int st = uimm (instr, 4, 0);

  FRegister a;
  u_int64_t addr = aarch64_get_reg_u64 (rn, SP_OK) + offset;
  aarch64_get_mem_long_double (addr, & a);
  return aarch64_set_FP_long_double (st, a);
}

// TODO store 8 bit unscaled signed 9 bit
// TODO store 16 bit unscaled signed 9 bit


// 1 source

// float absolute value
static StatusCode
fabss (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  float value = aarch64_get_FP_float (sn);
  return aarch64_set_FP_float (sd, fabsf (value));
}

// double absolute value
static StatusCode
fabsd (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  double value = aarch64_get_FP_double (sn);
  return aarch64_set_FP_double (sd, fabs (value));
}

// float negative value
static StatusCode
fnegs (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_float (sd, - aarch64_get_FP_float (sn));
}

// double negative value
static StatusCode
fnegd (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_double (sd, - aarch64_get_FP_double (sn));
}

// float square root
static StatusCode
fsqrts (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_float (sd, sqrt (aarch64_get_FP_float (sn)));
}

// double square root
static StatusCode
fsqrtd (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_double (sd, sqrt (aarch64_get_FP_double (sn)));
}

// convert double to float
static StatusCode
fcvtds (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_float (sd, (float) aarch64_get_FP_double (sn));
}

// convert float to double
static StatusCode
fcvtsd (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_double (sd, (double) aarch64_get_FP_float (sn));
}

static StatusCode
do_FRINT (void)
{
  // instr[31,23] = 0001 1110 0
  // instr[22]    = single(0)/double(1)
  // instr[21,18] = 1001
  // instr[17,15] = rounding mode
  // instr[14,10] = 10000
  // instr[9,5]   = source
  // instr[4,0]   = dest

  NYI_assert (31, 23, 0x03C);
  NYI_assert (21, 18, 0x9);
  NYI_assert (14, 10, 0x10);

  unsigned rs = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  unsigned int rmode = uimm (instr, 17, 15);

  if (rmode == 6 || rmode == 7)
    /* FIXME: Add support for rmode == 6 exactness check.  */
    rmode = uimm (aarch64_get_FPSR (), 23, 22);
  
  if (uimm (instr, 22, 22))
    {
      double val = aarch64_get_FP_double (rs);

      switch (rmode)
	{
	case 0: /* mode N: nearest or even.  */
	  {
	    double rval = round (val);
	    
	    if (val - rval == 0.5)
	      {
		if (((rval / 2.0) * 2.0) != rval)
		  rval += 1.0;
	      }
	
	    return aarch64_set_FP_double (rd, round (val));
	  }
	  
	case 1: /* mode P: towards +inf.  */
	  if (val < 0.0)
	    return aarch64_set_FP_double (rd, trunc (val));
	  return aarch64_set_FP_double (rd, round (val));
	    
	case 2: /* mode M: towards -inf.  */
	  if (val < 0.0)
	    return aarch64_set_FP_double (rd, round (val));
	  return aarch64_set_FP_double (rd, trunc (val));

	case 3: /* mode Z: towards 0.  */
	  return aarch64_set_FP_double (rd, trunc (val));

	case 4: /* mode A: away from 0.  */
	  return aarch64_set_FP_double (rd, round (val));

	case 6: /* mode X: use FPCR with exactness check.  */
	case 7: /* mode I: use FPCR mode.  */
	  return_NYI;

	default:
	  return_UNALLOC;
	}
    }

  float val = aarch64_get_FP_float (rs);

  switch (rmode)
    {
    case 0: /* mode N: nearest or even.  */
      {
	float rval = roundf (val);

	if (val - rval == 0.5)
	  {
	    if (((rval / 2.0) * 2.0) != rval)
	      rval += 1.0;
	  }
	
	return aarch64_set_FP_float (rd, rval);
      }

    case 1: /* mode P: towards +inf.  */
      if (val < 0.0)
	return aarch64_set_FP_float (rd, truncf (val));
      return aarch64_set_FP_float (rd, roundf (val));

    case 2: /* mode M: towards -inf.  */
      if (val < 0.0)
	return aarch64_set_FP_float (rd, truncf (val));
      return aarch64_set_FP_float (rd, roundf (val));

    case 3: /* mode Z: towards 0.  */
      return aarch64_set_FP_float (rd, truncf (val));

    case 4: /* mode A: away from 0.  */
      return aarch64_set_FP_float (rd, roundf (val));

    case 6: /* mode X: use FPCR with exactness check.  */
    case 7: /* mode I: use FPCR mode.  */
      report_and_die (INTERNAL_ERROR);

    default:
      return_UNALLOC;
    }
}

static StatusCode
dexSimpleFPDataProc1Source (void)
{
  // instr[31]    ==> M : 0 ==> OK, 1 ==> UNALLOC
  // instr[30]    = 0
  // instr[29]    ==> S :  0 ==> OK, 1 ==> UNALLOC
  // instr[28,25] = 1111 
  // instr[24]    = 0
  // instr[23,22] ==> type : 00 ==> source is single,
  //                         01 ==> source is double
  //                         10 ==> UNALLOC
  //                         11 ==> UNALLOC or source is half
  // instr[21]    = 1
  // instr[20,15] ==> opcode : with type 00 or 01
  //                           000000 ==> FMOV, 000001 ==> FABS,
  //                           000010 ==> FNEG, 000011 ==> FSQRT,
  //                           000100 ==> UNALLOC, 000101 ==> FCVT,(to single/double)
  //                           000110 ==> UNALLOC, 000111 ==> FCVT (to half)
  //                           001000 ==> FRINTN, 001001 ==> FRINTP,
  //                           001010 ==> FRINTM, 001011 ==> FRINTZ,
  //                           001100 ==> FRINTA, 001101 ==> UNALLOC
  //                           001110 ==> FRINTX, 001111 ==> FRINTI
  //                           with type 11
  //                           000100 ==> FCVT (half-to-single)
  //                           000101 ==> FCVT (half-to-double)
  // instr[14,10] = 10000

  u_int32_t M_S = (uimm (instr, 31, 31) << 1) | uimm (instr, 29, 29);
  if (M_S != 0)
    return_UNALLOC;

  u_int32_t type   = uimm (instr, 23, 22);
  u_int32_t opcode = uimm (instr, 20, 15);

  if (type == 3)
    {
      if (opcode == 4 || opcode == 5)
	return_NYI;
      else
	return_UNALLOC;
    }

  if (type == 2)
    return_UNALLOC;

  switch (opcode)
    {
    case 0:
      if (type)
	return ffmovd ();
      return ffmovs ();

    case 1:
      if (type)
	return fabsd ();
      return fabss ();

    case 2:
      if (type)
	return fnegd ();
      return fnegs ();

    case 3:
      if (type)
	return fsqrtd ();
      return fsqrts ();

    case 4:
      if (type)
	return fcvtds ();
      return_UNALLOC;

    case 5:
      if (type)
	return_UNALLOC;
      return fcvtsd ();

    case 8:		// FRINTN etc
    case 9:
    case 10:
    case 11:
    case 12:
    case 14:
    case 15:
       return do_FRINT ();

    case 7:		// FCVT double/single to half precision
    case 13:
      return_NYI;

    default:
      return_UNALLOC;
    }
}

// 32 bit signed int to float
static StatusCode
scvtf32 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_float (sd, (float) aarch64_get_reg_s32 (rn, NO_SP));
}

// signed int to float
static StatusCode
scvtf (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_float (sd, (float) aarch64_get_reg_s64 (rn, NO_SP));
}

// 32 bit signed int to double
static StatusCode
scvtd32 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_double (sd, (double) aarch64_get_reg_s32 (rn, NO_SP));
}

// signed int to double
static StatusCode
scvtd (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned sd = uimm (instr, 4, 0);

  return aarch64_set_FP_double (sd, (double) aarch64_get_reg_s64 (rn, NO_SP));
}

static const float  FLOAT_INT_MAX   = (float)  INT_MAX;
static const float  FLOAT_INT_MIN   = (float)  INT_MIN;
static const double DOUBLE_INT_MAX  = (double) INT_MAX;
static const double DOUBLE_INT_MIN  = (double) INT_MIN;
static const float  FLOAT_LONG_MAX  = (float)  LONG_MAX;
static const float  FLOAT_LONG_MIN  = (float)  LONG_MIN;
static const double DOUBLE_LONG_MAX = (double) LONG_MAX;
static const double DOUBLE_LONG_MIN = (double) LONG_MIN;

/* Check for FP exception conditions:
     NaN raises IO
     Infinity raises IO
     Out of Range raises IO and IX and saturates value
     Denormal raises ID and IX and sets to zero.  */
#define RAISE_EXCEPTIONS(F, VALUE, FTYPE, ITYPE)	\
  do							\
    {							\
      switch (fpclassify (F))				\
	{						\
	case FP_INFINITE:				\
	case FP_NAN:					\
	  aarch64_set_FPSR (IO);			\
	  if (signbit (F))				\
	    VALUE = ITYPE##_MAX;			\
	  else						\
	    VALUE = ITYPE##_MIN;			\
	  break;					\
							\
	case FP_NORMAL:					\
	  if (F >= FTYPE##_##ITYPE##_MAX)		\
	    {						\
	      aarch64_set_FPSR_bits (IO | IX, IO | IX);	\
	      VALUE = ITYPE##_MAX;			\
	    }						\
	  else if (F <= FTYPE##_##ITYPE##_MIN)		\
	    {						\
	      aarch64_set_FPSR_bits (IO | IX, IO | IX);	\
	      VALUE = ITYPE##_MIN;			\
	    }						\
	  break;					\
							\
	case FP_SUBNORMAL:				\
	  aarch64_set_FPSR_bits (IO | IX | ID, IX | ID);\
	  VALUE = 0;					\
	  break;					\
							\
	default:					\
	case FP_ZERO:					\
	  VALUE = 0;					\
	  break;					\
	}						\
    }							\
  while (0)
  
// 32 bit convert float to signed int truncate towards zero
static StatusCode
fcvtszs32 (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // TODO : check that this rounds toward zero
  float   f = aarch64_get_FP_float (sn);
  int32_t value = (int32_t) f;
  RAISE_EXCEPTIONS (f, value, FLOAT, INT);

  // avoid sign extension to 64 bit;
  return aarch64_set_reg_u64 (rd, NO_SP, (u_int32_t) value);
}

// 64 bit convert float to signed int truncate towards zero
static StatusCode
fcvtszs (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  float f = aarch64_get_FP_float (sn);
  int64_t value = (int64_t) f;
  RAISE_EXCEPTIONS (f, value, FLOAT, LONG);

  return aarch64_set_reg_s64 (rd, NO_SP, value);
}

// 32 bit convert double to signed int truncate towards zero
static StatusCode
fcvtszd32 (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // TODO : check that this rounds toward zero
  double   d = aarch64_get_FP_double (sn);
  int32_t  value = (int32_t) d;
  RAISE_EXCEPTIONS (d, value, DOUBLE, INT);

  // avoid sign extension to 64 bit;
  return aarch64_set_reg_u64 (rd, NO_SP, (u_int32_t) value);
}

// 64 bit convert double to signed int truncate towards zero
static StatusCode
fcvtszd (void)
{
  unsigned sn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // TODO : check that this rounds toward zero
  double  d = aarch64_get_FP_double (sn);
  int64_t value;
  value = (int64_t) d;

  RAISE_EXCEPTIONS (d, value, DOUBLE, LONG);

  return aarch64_set_reg_s64 (rd, NO_SP, value);
}

static StatusCode
do_fcvtzu (void)
{
  // instr[31]    = size: 32-bit (0), 64-bit (1)
  // instr[30,23] = 00111100
  // instr[22]    = type: single (0)/ double (1)
  // instr[21]    = enable (0)/disable(1) precision
  // instr[20,16] = 11001
  // instr[15,10] = precision
  // instr[9,5]   = Rs
  // instr[4,0]   = Rd

  NYI_assert (30, 23, 0x3C);
  NYI_assert (20, 16, 0x19);
  
  unsigned rs = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  if (uimm (instr, 21, 21) != 1)
    /* Convert to fixed point.  */
    return_NYI;

  if (uimm (instr, 31, 31))
    {
      // Convert to unsigned 64-bit integer.
      if (uimm (instr, 22, 22))
	{
	  double  d = aarch64_get_FP_double (rs);
	  u_int64_t value = (u_int64_t) d;

	  /* Do not raise an exception if we have reached ULONG_MAX.  */
	  if (value != (1UL << 63))
	    RAISE_EXCEPTIONS (d, value, DOUBLE, LONG);
	  return aarch64_set_reg_u64 (rd, NO_SP, value);
	}
      else
	{
	  float  f = aarch64_get_FP_float (rs);
	  u_int64_t value = (u_int64_t) f;

	  /* Do not raise an exception if we have reached ULONG_MAX.  */
	  if (value != (1UL << 63))
	    RAISE_EXCEPTIONS (f, value, FLOAT, LONG);
	  return aarch64_set_reg_u64 (rd, NO_SP, value);
	}
    }
  else
    {
      u_int32_t value;

      // Convert to unsigned 32-bit integer.
      if (uimm (instr, 22, 22))
	{
	  double  d = aarch64_get_FP_double (rs);

	  value = (u_int32_t) d;
	  /* Do not raise an exception if we have reached UINT_MAX.  */
	  if (value != (1UL << 31))
	    RAISE_EXCEPTIONS (d, value, DOUBLE, INT);
	}
      else
	{
	  float  f = aarch64_get_FP_float (rs);

	  value = (u_int32_t) f;
	  /* Do not raise an exception if we have reached UINT_MAX.  */
	  if (value != (1UL << 31))
	    RAISE_EXCEPTIONS (f, value, FLOAT, INT);
	}

      return aarch64_set_reg_u64 (rd, NO_SP, value);
    }
}

static StatusCode
do_UCVTF (void)
{
  // instr[31]    = size: 32-bit (0), 64-bit (1)
  // instr[30,23] = 001 1110 0
  // instr[22]    = type: single (0)/ double (1)
  // instr[21]    = enable (0)/disable(1) precision
  // instr[20,16] = 0 0011
  // instr[15,10] = precision
  // instr[9,5]   = Rs
  // instr[4,0]   = Rd

  NYI_assert (30, 23, 0x3C);
  NYI_assert (20, 16, 0x03);
  
  unsigned rs = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  if (uimm (instr, 21, 21) != 1)
    return_NYI;

  // FIXME: Add exception raising.
  if (uimm (instr, 31, 31))
    {
      u_int64_t value = aarch64_get_reg_u64 (rs, NO_SP);

      if (uimm (instr, 22, 22))
	return aarch64_set_FP_double (rd, (double) value);

      return aarch64_set_FP_float (rd, (float) value);
    }
  else
    {
      u_int32_t value =  aarch64_get_reg_u32 (rs, NO_SP);

      if (uimm (instr, 22, 22))
	return aarch64_set_FP_double (rd, (double) value);

      return aarch64_set_FP_float (rd, (float) value);
    }
}

static StatusCode
float_vector_move (void)
{
  // instr[31,17] == 100 1111 0101 0111
  // instr[16]    ==> direction 0=> to GR, 1=> from GR
  // instr[15,10] => ???
  // instr[9,5]   ==> source
  // instr[4,0]   ==> dest

  NYI_assert (31, 17, 0x4F57);

  if (uimm (instr, 15, 10) != 0)
    return_UNALLOC;

  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  if (uimm (instr, 16, 16))
    return aarch64_set_vec_u64 (rd, 1, aarch64_get_reg_u64 (rn, NO_SP));
  
  return aarch64_set_reg_u64 (rd, NO_SP, aarch64_get_vec_u64 (rn, 1));
}

static StatusCode
dexSimpleFPIntegerConvert (void)
{
  // instr[31]    = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30     = 0
  // instr[29]    = S :  0 ==> OK, 1 ==> UNALLOC
  // instr[28,25] = 1111 
  // instr[24]    = 0
  // instr[23,22] = type : 00 ==> single, 01 ==> double, 1x ==> UNALLOC
  // instr[21]    = 1
  // instr[20,19] = rmode
  // instr[18,16] = opcode 
  // instr[15,10] = 10 0000

  if (uimm (instr, 31, 17) == 0x4F57)
    return float_vector_move ();

  u_int32_t size = uimm (instr, 31, 31);
  u_int32_t S = uimm (instr, 29, 29);
  if (S != 0)
    return_UNALLOC;

  u_int32_t type = uimm (instr, 23, 22);
  if (type > 1)
    return_UNALLOC;

  u_int32_t rmode_opcode = uimm (instr, 20, 16);
  u_int32_t size_type = (size << 1) | type; // 0==32f, 1==32d, 2==64f, 3==64d

  switch (rmode_opcode)
    {
    case 2:			// SCVTF
      switch (size_type)
	{
	case 0: return scvtf32 ();
	case 1: return scvtd32 ();
	case 2: return scvtf ();
	case 3: return scvtd ();
	default: return report_and_die (INTERNAL_ERROR);
	}

    case 6:			// FMOV GR, Vec
      switch (size_type)
	{
	case 0:  return gfmovs ();
	case 3:  return gfmovd ();
	default: return_UNALLOC;
	}

    case 7:			// FMOV vec, GR
      switch (size_type)
	{
	case 0:  return fgmovs ();
	case 3:  return fgmovd ();
	default: return_UNALLOC;
	}

    case 24:			// FCVTZS
      switch (size_type)
	{
	case 0: return fcvtszs32 ();
	case 1: return fcvtszd32 ();
	case 2: return fcvtszs ();
	case 3: return fcvtszd ();
	default: return report_and_die (INTERNAL_ERROR);
	}

    case 25: return do_fcvtzu ();
    case 3:  return do_UCVTF ();

    case 0:	// FCVTNS
    case 1:	// FCVTNU
    case 4:	// FCVTAS
    case 5:	// FCVTAU
    case 8:	// FCVPTS
    case 9:	// FCVTPU
    case 16:	// FCVTMS
    case 17:	// FCVTMU
    default:
      return_NYI;
    }
}

static StatusCode
set_flags_for_float_compare (float fvalue1, float fvalue2)
{
  if (isnan (fvalue1) || isnan (fvalue2))
    return aarch64_set_CPSR (C|V);

  float result = fvalue1 - fvalue2;
  u_int32_t flags;

  if (result == 0.0)
    flags = Z|C;
  else if (result < 0)
    flags = N;
  else // (result > 0)
    flags = C;
  
  return aarch64_set_CPSR (flags);
}

static StatusCode
fcmps (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);

  float fvalue1 = aarch64_get_FP_float (sn);
  float fvalue2 = aarch64_get_FP_float (sm);

  return set_flags_for_float_compare (fvalue1, fvalue2);
}

// float compare to zero -- Invalid Operation exception only on signaling NaNs.
static StatusCode
fcmpzs (void)
{
  unsigned sn = uimm (instr,  9,  5);
  float fvalue1 = aarch64_get_FP_float (sn);

  return set_flags_for_float_compare (fvalue1, 0.0f);
}

// float compare -- Invalid Operation exception on all NaNs.
static StatusCode
fcmpes (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);

  float fvalue1 = aarch64_get_FP_float (sn);
  float fvalue2 = aarch64_get_FP_float (sm);

  return set_flags_for_float_compare (fvalue1, fvalue2);
}

// float compare to zero -- Invalid Operation exception on all NaNs.
static StatusCode
fcmpzes (void)
{
  unsigned sn = uimm (instr,  9,  5);
  float fvalue1 = aarch64_get_FP_float (sn);

  return set_flags_for_float_compare (fvalue1, 0.0f);
}

static StatusCode
set_flags_for_double_compare (double dval1, double dval2)
{
  if (isnan (dval1) || isnan (dval2))
    return aarch64_set_CPSR (C|V);

  u_int32_t flags;
  double result = dval1 - dval2;

  if (result == 0.0)
    flags = Z|C;
  else if (result < 0)
    flags = N;
  else // (result > 0)
    flags = C;

  return aarch64_set_CPSR (flags);
}

// double compare -- Invalid Operation exception only on signaling NaNs.
static StatusCode
fcmpd (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);

  double dvalue1 = aarch64_get_FP_double (sn);
  double dvalue2 = aarch64_get_FP_double (sm);

  return set_flags_for_double_compare (dvalue1, dvalue2);
}

// double compare to zero -- Invalid Operation exception only on signaling NaNs.
static StatusCode
fcmpzd (void)
{
  unsigned sn = uimm (instr,  9,  5);
  double dvalue1 = aarch64_get_FP_double (sn);

  return set_flags_for_double_compare (dvalue1, 0.0);
}

// double compare -- Invalid Operation exception on all NaNs.
static StatusCode
fcmped (void)
{
  unsigned sm = uimm (instr, 20, 16);
  unsigned sn = uimm (instr,  9,  5);

  double dvalue1 = aarch64_get_FP_double (sn);
  double dvalue2 = aarch64_get_FP_double (sm);

  return set_flags_for_double_compare (dvalue1, dvalue2);
}

// double compare to zero -- Invalid Operation exception on all NaNs.
static StatusCode
fcmpzed (void)
{
  unsigned sn = uimm (instr,  9,  5);
  double dvalue1 = aarch64_get_FP_double (sn);

  return set_flags_for_double_compare (dvalue1, 0.0);
}

static StatusCode
dexSimpleFPCompare (void)
{
  // assert instr[28,25] == 1111 and instr[30:24:21:13,10] = 0011000
  // instr[31] = M : 0 ==> OK, 1 ==> UNALLOC
  // instr[29] ==> S :  0 ==> OK, 1 ==> UNALLOC
  // instr[23,22] ==> type : 0 ==> single, 01 ==> double, 1x ==> UNALLOC
  // instr[15,14] ==> op : 00 ==> OK, ow ==> UNALLOC
  // instr[4,0] ==> opcode2 : 00000 ==> FCMP, 10000 ==> FCMPE,
  //                          01000 ==> FCMPZ, 11000 ==> FCMPEZ,
  //                          ow ==> UNALLOC

  u_int32_t M_S = (uimm (instr, 31, 31) << 1) | uimm (instr, 29, 29);
  if (M_S != 0)
    return_UNALLOC;

  u_int32_t type = uimm (instr, 23, 22);
  if (type > 1)
    return_UNALLOC;

  u_int32_t op = uimm (instr, 15, 14);
  if (op != 0)
    return_UNALLOC;

  u_int32_t op2_2_0 = uimm (instr, 2, 0);
  if (op2_2_0 != 0)
    return_UNALLOC;

  // dispatch on type and top 2 bits of opcode
  u_int32_t dispatch = (type << 2) | uimm (instr, 4, 3);

  switch (dispatch)
    {
    case 0: return fcmps ();
    case 1: return fcmpzs ();
    case 2: return fcmpes ();
    case 3: return fcmpzes ();
    case 4: return fcmpd ();
    case 5: return fcmpzd ();
    case 6: return fcmped ();
    case 7: return fcmpzed ();
    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

static StatusCode
do_scalar_FADDP (void)
{
  // instr [31,23] = 011111100
  // instr [22]    = single(0)/double(1)
  // instr [21,10] = 1100 0011 0110 
  // instr [9,5]   = Fn
  // instr [4,0]   = Fd

  NYI_assert (31, 23, 0x0FC);
  NYI_assert (21, 10, 0xC36);

  unsigned Fn = uimm (instr, 9, 5);
  unsigned Fd = uimm (instr, 4, 0);

  if (uimm (instr, 22, 22))
    {
      double val1 = aarch64_get_vec_double (Fn, 0);
      double val2 = aarch64_get_vec_double (Fn, 1);
  
      return aarch64_set_FP_double (Fd, val1 + val2);
    }
  else
    {
      float val1 = aarch64_get_vec_float (Fn, 0);
      float val2 = aarch64_get_vec_float (Fn, 1);
  
      return aarch64_set_FP_float (Fd, val1 + val2);
    }
}

// Floating point absolute difference.

static StatusCode
do_scalar_FABD (void)
{
  // instr [31,23] = 0111 1110 1
  // instr [22]    = float(0)/double(1)
  // instr [21]    = 1
  // instr [20,16] = Rm
  // instr [15,10] = 1101 01
  // instr [9, 5]  = Rn
  // instr [4, 0]  = Rd

  NYI_assert (31, 23, 0x0FD);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 10, 0x35);

  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  if (uimm (instr, 22, 22))
    return aarch64_set_FP_double (rd,
				   fabs (aarch64_get_FP_double (rn)
					 - aarch64_get_FP_double (rm)));

  return aarch64_set_FP_float (rd,
				fabsf (aarch64_get_FP_float (rn)
				       - aarch64_get_FP_float (rm)));
}

static StatusCode
do_scalar_CMGT (void)
{
  // instr [31,21] = 0101 1110 111
  // instr [20,16] = Rm
  // instr [15,10] = 00 1101
  // instr [9, 5]  = Rn
  // instr [4, 0]  = Rd

  NYI_assert (31, 21, 0x2F7);
  NYI_assert (15, 10, 0x0D);

  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_vec_u64 (rd, 0,
			      aarch64_get_vec_u64 (rn, 0) >
			      aarch64_get_vec_u64 (rm, 0) ? -1L : 0L);
}

static StatusCode
do_scalar_USHR (void)
{
  // instr [31,23] = 0111 1111 0
  // instr [22,16] = shift amount
  // instr [15,10] = 0000 01
  // instr [9, 5]  = Rn
  // instr [4, 0]  = Rd

  NYI_assert (31, 23, 0x0FE);
  NYI_assert (15, 10, 0x01);

  unsigned amount = 128 - uimm (instr, 22, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_vec_u64 (rd, 0,
			      aarch64_get_vec_u64 (rn, 0) >> amount);
}

static StatusCode
do_scalar_SHL (void)
{
  // instr [31,23] = 0111 1101 0
  // instr [22,16] = shift amount
  // instr [15,10] = 0101 01
  // instr [9, 5]  = Rn
  // instr [4, 0]  = Rd

  NYI_assert (31, 23, 0x0BE);
  NYI_assert (15, 10, 0x15);

  if (uimm (instr, 22, 22) == 0)
    return_UNALLOC;

  unsigned amount = uimm (instr, 22, 16) - 64;
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_vec_u64 (rd, 0,
			      aarch64_get_vec_u64 (rn, 0) << amount);
}

// FCMEQ FCMGT FCMGE
static StatusCode
do_scalar_FCM (void)
{
  // instr [31,30] = 01
  // instr [29]    = U
  // instr [28,24] = 1 1110
  // instr [23]    = E
  // instr [22]    = size
  // instr [21]    = 1
  // instr [20,16] = Rm
  // instr [15,12] = 1110
  // instr [11]    = AC
  // instr [10]    = 1
  // instr [9, 5]  = Rn
  // instr [4, 0]  = Rd

  NYI_assert (31, 30, 1);
  NYI_assert (28, 24, 0x1E);
  NYI_assert (21, 21, 1);
  NYI_assert (15, 12, 0xE);
  NYI_assert (10, 10, 1);

  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  unsigned EUac = (uimm (instr, 23, 23) << 2) | (uimm (instr, 29, 29) << 1) | uimm (instr, 11, 11);
  unsigned result;
  
  if (uimm (instr, 22, 22))
    {
      double val1 = aarch64_get_FP_double (rn);
      double val2 = aarch64_get_FP_double (rm);

      switch (EUac)
	{
	case 0: /* 000 */ result = val1 == val2; break;      

	case 3: /* 011 */ val1 = fabs (val1); val2 = fabs (val2); /* Fall through. */
	case 2: /* 010 */ result = val1 >= val2; break;

	case 7: /* 111 */ val1 = fabs (val1); val2 = fabs (val2); /* Fall through. */
	case 6: /* 110 */ result = val1 > val2; break;

	default:
	  return_UNALLOC;
	}
  
      return aarch64_set_vec_u32 (rd, 0, result ? -1 : 0);
    }

  float val1 = aarch64_get_FP_float (rn);
  float val2 = aarch64_get_FP_float (rm);

  switch (EUac)
    {
    case 0: /* 000 */ result = val1 == val2; break;      

    case 3: /* 011 */ val1 = fabsf (val1); val2 = fabsf (val2); /* Fall through. */
    case 2: /* 010 */ result = val1 >= val2; break;

    case 7: /* 111 */ val1 = fabsf (val1); val2 = fabsf (val2); /* Fall through. */
    case 6: /* 110 */ result = val1 > val2; break;

    default:
      return_UNALLOC;
    }
  
  return aarch64_set_vec_u32 (rd, 0, result ? -1 : 0);
}

// An alias of DUP.
static StatusCode
do_scalar_MOV (void)
{
  // instr [31,21] = 0101 1110 000
  // instr [20,16] = imm5
  // instr [15,10] = 0000 01
  // instr [9, 5]  = Rn
  // instr [4, 0]  = Rd

  NYI_assert (31, 21, 0x2F0);
  NYI_assert (15, 10, 0x01);

  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  unsigned index;

  if (uimm (instr, 16, 16))
    {
      // 8-bit
      index = uimm (instr, 20, 17);
      return aarch64_set_vec_u8 (rd, 0, aarch64_get_vec_u8 (rn, index));
    }
  else if (uimm (instr, 17, 17))
    {
      // 16-bit
      index = uimm (instr, 20, 18);
      return aarch64_set_vec_u16 (rd, 0, aarch64_get_vec_u16 (rn, index));
    }
  else if (uimm (instr, 18, 18))
    {
      // 32-bit
      index = uimm (instr, 20, 19);
      return aarch64_set_vec_u32 (rd, 0, aarch64_get_vec_u32 (rn, index));
    }
  else if (uimm (instr, 19, 19))
    {
      // 64-bit.
      index = uimm (instr, 20, 20);
      return aarch64_set_vec_u64 (rd, 0, aarch64_get_vec_u64 (rn, index));
    }
  else
    return_UNALLOC;
}

static StatusCode
do_double_add (void)
{
  // instr [28,25] = 1111

  switch (uimm (instr, 31, 23))
    {
    case 0xBC:
      switch (uimm (instr, 15, 10))
	{
	case 0x01: return do_scalar_MOV ();
	case 0x39: return do_scalar_FCM ();
	case 0x3B: return do_scalar_FCM ();
	}      
      break;

    case 0xBE: return do_scalar_SHL ();

    case 0xFC:
      switch (uimm (instr, 15, 10))
	{
	case 0x36: return do_scalar_FADDP ();
	case 0x39: return do_scalar_FCM ();
	case 0x3B: return do_scalar_FCM ();
	}      
      break;
      
    case 0xFD:
      switch (uimm (instr, 15, 10))
	{
	case 0x0D: return do_scalar_CMGT ();
	case 0x35: return do_scalar_FABD ();
	case 0x39: return do_scalar_FCM ();
	case 0x3B: return do_scalar_FCM ();
	default:
	  return_NYI;
	}

    case 0xFE: return do_scalar_USHR ();
    default:
      break;
    }

  // instr [31,21] = 0101 1110 111
  // instr [20,16] = Fn
  // instr [15,10] = 1000 01
  // instr [9,5]   = Fm
  // instr [4,0]   = Fd
  if (uimm (instr, 31, 21) != 0x2F7
      || uimm (instr, 15, 10) != 0x21)
    return_NYI;
  
  unsigned Fd = uimm (instr, 4, 0);
  unsigned Fm = uimm (instr, 9, 5);
  unsigned Fn = uimm (instr, 20, 16);

  double val1 = aarch64_get_FP_double (Fm);
  double val2 = aarch64_get_FP_double (Fn);
  
  return aarch64_set_FP_double (Fd, val1 + val2);
}

static StatusCode
dexAdvSIMD1 (void)
{
  // instr [28,25] = 1 111

  // we are currently only interested in the basic
  // scalar fp routines which all have bit 30 = 0
  if (uimm (instr, 30, 30))
    return do_double_add ();

  // instr[24] is set for FP data processing 3-source and clear for
  // all other basic scalar fp instruction groups
  if (uimm (instr, 24, 24))
    return dexSimpleFPDataProc3Source ();

  // instr[21] is clear for floating <-> fixed conversions and set for
  // all other basic scalar fp instruction groups
  if (!uimm (instr, 21, 21))
    return dexSimpleFPFixedConvert ();

  // instr[11,10] : 01 ==> cond compare, 10 ==> Data Proc 2 Source
  //                11 ==> cond select,  00 ==> other
  switch (uimm (instr, 11, 10))
    {
    case 1: return dexSimpleFPCondCompare ();
    case 2: return dexSimpleFPDataProc2Source ();
    case 3: return dexSimpleFPCondSelect ();
    default:
      break;
    }

  // now an ordered cascade of tests
  // FP immediate has instr[12] == 1
  // FP compare has instr[13] == 1
  // FP Data Proc 1 Source has instr[14] == 1
  // FP floating <--> integer conversions has instr[15] == 0
  if (uimm (instr, 12, 12))
    return dexSimpleFPImmediate ();

  if (uimm (instr, 13, 13))
    return dexSimpleFPCompare ();

  if (uimm (instr, 14, 14))
    return dexSimpleFPDataProc1Source ();

  if (!uimm (instr, 15, 15))
    return dexSimpleFPIntegerConvert ();

  // if we get here the instr[15] == 1 which means UNALLOC
  return_UNALLOC;
}

// PC relative addressing

static StatusCode
pcadr (void)
  // instr[31] = op : 0 ==> ADR, 1 ==> ADRP
  // instr[30,29] = immlo
  // instr[23,5] = immhi
{
  unsigned rd = uimm (instr, 4, 0);
  u_int32_t isPage = uimm (instr, 31, 31);
  union { int64_t u64; u_int64_t s64; } imm;

  imm.s64 = simm64 (instr, 23, 5);
  u_int64_t offset = imm.u64;
  offset = (offset << 2) | uimm (instr, 30, 29);

  u_int64_t address = aarch64_get_PC ();

  if (isPage)
    {
      offset <<= 12;
      address &= ~0xfff;
    }

  return aarch64_set_reg_u64 (rd, NO_SP, address + offset);
}

// specific decode and execute for group Data Processing Immediate

static StatusCode
dexPCRelAddressing (void)
{
  // assert instr[28,24] = 10000
  return pcadr ();
}

// immediate logical
// the bimm32/64 argument is constructed by replicating a 2, 4, 8,
// 16, 32 or 64 bit sequence pulled out at decode and possibly
// inverting it.
//
// n.b. the output register (dest) can normally be Xn or SP.
// the exception occurs for flag setting instructions which may
// only use Xn for the output (dest). the input register can
// never be SP.

// 32 bit and immediate
static StatusCode
and32 (u_int32_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK,  aarch64_get_reg_u32 (rn, NO_SP) & bimm);
}

// 64 bit and immediate
static StatusCode
and64 (u_int64_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK,  aarch64_get_reg_u64 (rn, NO_SP) & bimm);
}

// 32 bit and immediate set flags
static StatusCode
ands32 (u_int32_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t value2 = bimm;

  aarch64_set_reg_u64 (rd, NO_SP, value1 & value2);
  return set_flags_for_binop32 (value1 & value2);
}

// 64 bit and immediate set flags
static StatusCode
ands64 (u_int64_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t value2 = bimm;

  aarch64_set_reg_u64 (rd, NO_SP, value1 & value2);
  return set_flags_for_binop64 (value1 & value2);
}

// 32 bit exclusive or immediate
static StatusCode
eor32 (u_int32_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK,  aarch64_get_reg_u32 (rn, NO_SP) ^ bimm);
}

// 64 bit exclusive or immediate
static StatusCode
eor64 (u_int64_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u64 (rn, NO_SP) ^ bimm);
}

// 32 bit or immediate
static StatusCode
orr32 (u_int32_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u32 (rn, NO_SP) | bimm);
}

// 64 bit or immediate
static StatusCode
orr64 (u_int64_t bimm)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, SP_OK, aarch64_get_reg_u64 (rn, NO_SP) | bimm);
}

// logical shifted register
// these allow an optional LSL, ASR, LSR or ROR to the second source
// register with a count up to the register bit count
//
// n.b register args may not be SP.

// 32 bit AND shifted register
static StatusCode
and32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      & shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit AND shifted register
static StatusCode
and64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      & shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

// 32 bit AND shifted register setting flags
static StatusCode
ands32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t value2 = shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 & value2);
  return set_flags_for_binop32 (value1 & value2);
}

// 64 bit AND shifted register setting flags
static StatusCode
ands64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t value2 = shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 & value2);
  return set_flags_for_binop64 (value1 & value2);
}

// 32 bit BIC shifted register
static StatusCode
bic32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      & ~ shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit BIC shifted register
static StatusCode
bic64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      & ~ shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

// 32 bit BIC shifted register setting flags
static StatusCode
bics32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int32_t value1 = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t value2 = ~ shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 & value2);
  return set_flags_for_binop32 (value1 & value2);
}

// 64 bit BIC shifted register setting flags
static StatusCode
bics64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  u_int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t value2 = ~ shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count);

  aarch64_set_reg_u64 (rd, NO_SP, value1 & value2);
  return set_flags_for_binop64 (value1 & value2);
}

// 32 bit EON shifted register
static StatusCode
eon32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      ^ ~ shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit EON shifted register
static StatusCode
eon64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      ^ ~ shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

// 32 bit EOR shifted register
static StatusCode
eor32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      ^ shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit EOR shifted register
static StatusCode
eor64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      ^ shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

// 32 bit ORR shifted register
static StatusCode
orr32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      | shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit ORR shifted register
static StatusCode
orr64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      | shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

// 32 bit ORN shifted register
static StatusCode
orn32_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (rn, NO_SP)
			      | ~ shifted32 (aarch64_get_reg_u32 (rm, NO_SP), shift, count));
}

// 64 bit ORN shifted register
static StatusCode
orn64_shift (Shift shift, u_int32_t count)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (rn, NO_SP)
			      | ~ shifted64 (aarch64_get_reg_u64 (rm, NO_SP), shift, count));
}

static StatusCode
dexLogicalImmediate (void)
{
  // assert instr[28,23] = 1001000
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30,29] = op : 0 ==> AND, 1 ==> ORR, 2 ==> EOR, 3 ==> ANDS
  // instr[22] = N : used to construct immediate mask
  // instr[21,16] = immr
  // instr[15,10] = imms
  // instr[9,5] = Rn
  // instr[4,0] = Rd

  // 32 bit operations must have N = 0 or else we have an UNALLOC
  u_int32_t size = uimm (instr, 31, 31);
  u_int32_t N = uimm (instr, 22, 22);
  if (~size & N)
    return_UNALLOC;

  // u_int32_t immr = uimm (instr, 21, 16);
  // u_int32_t imms = uimm (instr, 15, 10);
  u_int32_t index = uimm (instr, 22, 10);
  u_int64_t bimm64 = LITable [index];
  u_int32_t dispatch = uimm (instr, 30, 29);

  if (!bimm64)
    return_UNALLOC;

  if (size == 0)
    {
      u_int32_t bimm = (u_int32_t) bimm64;

      switch (dispatch)
	{
	case 0: return and32 (bimm);
	case 1: return orr32 (bimm);
	case 2: return eor32 (bimm);
	case 3: return ands32 (bimm);
	}
    }
  else
    {
      switch (dispatch)
	{
	case 0: return and64 (bimm64);
	case 1: return orr64 (bimm64);
	case 2: return eor64 (bimm64);
	case 3: return ands64 (bimm64);
	}
    }
  return_UNALLOC;
}

// immediate move
// the uimm argument is a 16 bit value to be inserted into the
// target register the pos argument locates the 16 bit word in the
// dest register i.e. it is in {0, 1} for 32 bit and {0, 1, 2,
// 3} for 64 bit
//
// n.b register arg may not be SP so it should be
// accessed using the setGZRegisterXXX accessors.

// 32 bit move 16 bit immediate zero remaining shorts
static StatusCode
movz32 (u_int32_t val, u_int32_t pos)
{
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, val << (pos * 16));
}

// 64 bit move 16 bit immediate zero remaining shorts
static StatusCode
movz64 (u_int32_t val, u_int32_t pos)
{
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, ((u_int64_t) val) << (pos * 16));
}

// 32 bit move 16 bit immediate negated
static StatusCode
movn32 (u_int32_t val, u_int32_t pos)
{
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, ((val << (pos * 16)) ^ 0xffffffffU));
}

// 64 bit move 16 bit immediate negated
static StatusCode
movn64 (u_int32_t val, u_int32_t pos)
{
  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, ((((u_int64_t) val) << (pos * 16)) ^ 0xffffffffffffffffULL));
}

// 32 bit move 16 bit immediate keep remaining shorts
static StatusCode
movk32 (u_int32_t val, u_int32_t pos)
{
  unsigned rd = uimm (instr, 4, 0);
  u_int32_t current = aarch64_get_reg_u32 (rd, NO_SP);
  u_int32_t value = val << (pos * 16);
  u_int32_t mask = ~(0xffffU << (pos * 16));

  return aarch64_set_reg_u64 (rd, NO_SP, (value | (current & mask)));
}

// 64 bit move 16 it immediate keep remaining shorts
static StatusCode
movk64 (u_int32_t val, u_int32_t pos)
{
  unsigned rd = uimm (instr, 4, 0);
  u_int64_t current = aarch64_get_reg_u64 (rd, NO_SP);
  u_int64_t value = (u_int64_t) val << (pos * 16);
  u_int64_t mask = ~(0xffffULL << (pos * 16));

  return aarch64_set_reg_u64 (rd, NO_SP, (value | (current & mask)));
}

static StatusCode
dexMoveWideImmediate (void)
{
  // assert instr[28:23] = 100101
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30,29] = op : 0 ==> MOVN, 1 ==> UNALLOC, 2 ==> MOVZ, 3 ==> MOVK
  // instr[22,21] = shift : 00 == LSL#0, 01 = LSL#16, 10 = LSL#32, 11 = LSL#48
  // instr[20,5] = uimm16
  // instr[4,0] = Rd

  // n.b. the (multiple of 16) shift is applied by the called routine
  // we just pass the multiplier

  u_int32_t size = uimm (instr, 31, 31);
  u_int32_t op = uimm (instr, 30, 29);
  u_int32_t shift = uimm (instr, 22, 21);

  // 32 bit can only shift 0 or 1 lot of 16
  // anything else is an unallocated instruction
  if (size == 0 && (shift > 1))
    return_UNALLOC;

  if (op == 1)
    return_UNALLOC;

  u_int32_t imm = uimm (instr, 20, 5);

  if (size == 0)
    {
      if (op == 0)
	movn32 (imm, shift);
      else if (op == 2)
	movz32 (imm, shift);
      else
	movk32 (imm, shift);
    }
  else
    {
      if (op == 0)
	movn64 (imm, shift);
      else if (op == 2)
	movz64 (imm, shift);
      else
	movk64 (imm, shift);
    } 
  
  return status;
}

// bitfield operations
// these take a pair of bit positions r and s which are in {0..31}
// or {0..63} depending on the instruction word size
//
// n.b register args may not be SP.


// ok, we start with ubfm which just needs to pick
// some bits out of source zero the rest and write
// the result to dest. just need two logical shifts

// 32 bit bitfield move, left and right of affected zeroed
// if r <= s Wd<s-r:0> = Wn<s:r> else Wd<32+s-r,32-r> = Wn<s:0>.
static StatusCode
ubfm32 (u_int32_t r, u_int32_t s)
{
  unsigned rn = uimm (instr, 9, 5);
  u_int32_t value = aarch64_get_reg_u32 (rn, NO_SP);

  // pick either s+1-r or s+1 consecutive bits out of the original word
  if (r <= s)
    {
      // 31:...:s:xxx:r:...:0 ==> 31:...:s-r:xxx:0
      // we want only bits s:xxx:r at the bottom of the word
      // so we LSL bit s up to bit 31 i.e. by 31 - s
      // and then we LSR to bring bit 31 down to bit s - r i.e. by 31 + r - s
      value <<= 31 - s;
      value >>= 31 + r - s;
    }
  else
    {
      // 31:...:s:xxx:0 ==> 31:...:31-(r-1)+s:xxx:31-(r-1):...:0
      // we want only bits s:xxx:0 starting at it 31-(r-1)
      // so we LSL bit s up to bit 31 i.e. by 31 - s
      // and then we LSL to bring bit 31 down to 31-(r-1)+s i.e. by r - (s + 1)
      value <<= 31 - s;
      value >>= r - (s + 1);
    }

  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, value);
}

// 64 bit bitfield move, left and right of affected zeroed
// if r <= s Wd<s-r:0> = Wn<s:r> else Wd<64+s-r,64-r> = Wn<s:0>.
static StatusCode
ubfm (u_int32_t r, u_int32_t s)
{
  unsigned rn = uimm (instr, 9, 5);
  u_int64_t value = aarch64_get_reg_u64 (rn, NO_SP);

  if (r <= s)
    {
      // 63:...:s:xxx:r:...:0 ==> 63:...:s-r:xxx:0
      // we want only bits s:xxx:r at the bottom of the word
      // so we LSL bit s up to bit 63 i.e. by 63 - s
      // and then we LSR to bring bit 63 down to bit s - r i.e. by 63 + r - s
      value <<= 63 - s;
      value >>= 63 + r - s;
    }
  else
    {
      // 63:...:s:xxx:0 ==> 63:...:63-(r-1)+s:xxx:63-(r-1):...:0
      // we want only bits s:xxx:0 starting at it 63-(r-1)
      // so we LSL bit s up to bit 63 i.e. by 63 - s
      // and then we LSL to bring bit 63 down to 63-(r-1)+s i.e. by r - (s + 1)
      value <<= 63 - s;
      value >>= r - (s + 1);
    }

  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, value);
}

// the signed versions need to insert sign bits
// on the left of the inserted bit field. so we do
// much the same as the unsigned version except we
// use an arithmetic shift right -- this just means
// we need to operate on signed values

// 32 bit bitfield move, left of affected sign-extended, right zeroed
// if r <= s Wd<s-r:0> = Wn<s:r> else Wd<32+s-r,32-r> = Wn<s:0>.
static StatusCode
sbfm32 (u_int32_t r, u_int32_t s)
{
  unsigned rn = uimm (instr, 9, 5);
  // as per ubfm32 but use an ASR instead of an LSR
  int32_t value = aarch64_get_reg_s32 (rn, NO_SP);

  if (r <= s)
    {
      value <<= 31 - s;
      value >>= 31 + r - s;
    }
  else
    {
      value <<= 31 - s;
      value >>= r - (s + 1);
    }

  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, (u_int32_t) value);
}

// 64 bit bitfield move, left of affected sign-extended, right zeroed
// if r <= s Wd<s-r:0> = Wn<s:r> else Wd<64+s-r,64-r> = Wn<s:0>.
static StatusCode
sbfm (u_int32_t r, u_int32_t s)
{
  unsigned rn = uimm (instr, 9, 5);
  // asd per ubfm but use an ASR instead of an LSR
  int64_t value = aarch64_get_reg_s64 (rn, NO_SP);

  if (r <= s)
    {
      value <<= 63 - s;
      value >>= 63 + r - s;
    }
  else
    {
      value <<= 63 - s;
      value >>= r - (s + 1);
    }

  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_s64 (rd, NO_SP, value);
}

// finally, these versions leave non-affected bits
// as is. so we need to generate the bits as per
// ubfm and also generate a mask to pick the
// bits from the original and computed values

// 32 bit bitfield move, non-affected bits left as is
// if r <= s Wd<s-r:0> = Wn<s:r> else Wd<32+s-r,32-r> = Wn<s:0>.
static StatusCode
bfm32 (u_int32_t r, u_int32_t s)
{
  unsigned rn = uimm (instr, 9, 5);
  u_int32_t value = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t mask = -1;

  // pick either s+1-r or s+1 consecutive bits out of the original word
  if (r <= s)
    {
      // 31:...:s:xxx:r:...:0 ==> 31:...:s-r:xxx:0
      // we want only bits s:xxx:r at the bottom of the word
      // so we LSL bit s up to bit 31 i.e. by 31 - s
      // and then we LSR to bring bit 31 down to bit s - r i.e. by 31 + r - s
      value <<= 31 - s;
      value >>= 31 + r - s;
      // the mask must include the same bits
      mask <<= 31 - s;
      mask >>= 31 + r - s;
    }
  else
    {
      // 31:...:s:xxx:0 ==> 31:...:31-(r-1)+s:xxx:31-(r-1):...:0
      // we want only bits s:xxx:0 starting at it 31-(r-1)
      // so we LSL bit s up to bit 31 i.e. by 31 - s
      // and then we LSL to bring bit 31 down to 31-(r-1)+s i.e. by r - (s + 1)
      value <<= 31 - s;
      value >>= r - (s + 1);
      // the mask must include the same bits
      mask <<= 31 - s;
      mask >>= r - (s + 1);
    }

  unsigned rd = uimm (instr, 4, 0);
  u_int32_t value2 = aarch64_get_reg_u32 (rd, NO_SP);

  value2 &= ~mask;
  value2 |= value;

  return aarch64_set_reg_u64 (rd, NO_SP, (aarch64_get_reg_u32 (rd, NO_SP) & ~mask) | value);
}

// 64 bit bitfield move, non-affected bits left as is
// if r <= s Wd<s-r:0> = Wn<s:r> else Wd<64+s-r,64-r> = Wn<s:0>.
static StatusCode
bfm (u_int32_t r, u_int32_t s)
{
  unsigned rn = uimm (instr, 9, 5);
  u_int64_t value = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t mask = 0xffffffffffffffffULL;

  if (r <= s)
    {
      // 63:...:s:xxx:r:...:0 ==> 63:...:s-r:xxx:0
      // we want only bits s:xxx:r at the bottom of the word
      // so we LSL bit s up to bit 63 i.e. by 63 - s
      // and then we LSR to bring bit 63 down to bit s - r i.e. by 63 + r - s
      value <<= 63 - s;
      value >>= 63 + r - s;
      // the mask must include the same bits
      mask <<= 63 - s;
      mask >>= 63 + r - s;
    }
  else
    {
      // 63:...:s:xxx:0 ==> 63:...:63-(r-1)+s:xxx:63-(r-1):...:0
      // we want only bits s:xxx:0 starting at it 63-(r-1)
      // so we LSL bit s up to bit 63 i.e. by 63 - s
      // and then we LSL to bring bit 63 down to 63-(r-1)+s i.e. by r - (s + 1)
      value <<= 63 - s;
      value >>= r - (s + 1);
      // the mask must include the same bits
      mask <<= 63 - s;
      mask >>= r - (s + 1);
    }

  unsigned rd = uimm (instr, 4, 0);
  return aarch64_set_reg_u64 (rd, NO_SP, (aarch64_get_reg_u64 (rd, NO_SP) & ~mask) | value);
}

static StatusCode
dexBitfieldImmediate (void)
{
  // assert instr[28:23] = 100110
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30,29] = op : 0 ==> SBFM, 1 ==> BFM, 2 ==> UBFM, 3 ==> UNALLOC
  // instr[22] = N : must be 0 for 32 bit, 1 for 64 bit ow UNALLOC
  // instr[21,16] = immr : 0xxxxx for 32 bit, xxxxxx for 64 bit
  // instr[15,10] = imms :  0xxxxx for 32 bit, xxxxxx for 64 bit
  // instr[9,5] = Rn
  // instr[4,0] = Rd

  // 32 bit operations must have N = 0 or else we have an UNALLOC
  u_int32_t size = uimm (instr, 31, 31);
  u_int32_t N = uimm (instr, 22, 22);
  if (~size & N)
    return_UNALLOC;

  // 32 bit operations must have immr[5] = 0 and imms[5] = 0
  // or else we have an UNALLOC
  u_int32_t immr = uimm (instr, 21, 16);
  if (!size && uimm (immr, 5, 5))
    return_UNALLOC;

  u_int32_t imms = uimm (instr, 15, 10);
  if (!size && uimm (imms, 5, 5))
    return_UNALLOC;

  // switch on combined size and op
  u_int32_t dispatch = uimm (instr, 31, 29);
  switch (dispatch)
    {
    case 0: return sbfm32 (immr, imms);
    case 1: return bfm32 (immr, imms);
    case 2: return ubfm32 (immr, imms);
    case 4: return sbfm (immr, imms);
    case 5: return bfm (immr, imms);
    case 6: return ubfm (immr, imms);
    default: return_UNALLOC;
    }
}

static StatusCode
do_EXTR_32 (void)
{
  // instr[31:21] = 00010011100
  // instr[20,16] = Rm
  // instr[15,10] = imms :  0xxxxx for 32 bit
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd
  unsigned rm   = uimm (instr, 20, 16);
  unsigned imms = uimm (instr, 15, 10) & 31;
  unsigned rn   = uimm (instr,  9,  5);
  unsigned rd   = uimm (instr,  4,  0);
  u_int64_t val1;
  u_int64_t val2;

  val1 = aarch64_get_reg_u32 (rm, NO_SP);
  val1 >>= imms;
  val2 = aarch64_get_reg_u32 (rn, NO_SP);
  val2 <<= (32 - imms);
  return aarch64_set_reg_u64 (rd, NO_SP, val1 | val2);
}

static StatusCode
do_EXTR_64 (void)
{
  // instr[31:21] = 10010011100
  // instr[20,16] = Rm
  // instr[15,10] = imms
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd
  unsigned rm   = uimm (instr, 20, 16);
  unsigned imms = uimm (instr, 15, 10) & 63;
  unsigned rn   = uimm (instr,  9,  5);
  unsigned rd   = uimm (instr,  4,  0);
  u_int64_t val;

  val = aarch64_get_reg_u64 (rm, NO_SP);
  val >>= imms;
  val |= (aarch64_get_reg_u64 (rn, NO_SP) << (64 - imms));

  return aarch64_set_reg_u64 (rd, NO_SP, val);
}

static StatusCode
dexExtractImmediate (void)
{
  // assert instr[28:23] = 100111
  // instr[31]    = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30,29] = op21 : 0 ==> EXTR, 1,2,3 ==> UNALLOC
  // instr[22]    = N : must be 0 for 32 bit, 1 for 64 bit or UNALLOC
  // instr[21]    = op0 : must be 0 or UNALLOC
  // instr[20,16] = Rm
  // instr[15,10] = imms :  0xxxxx for 32 bit, xxxxxx for 64 bit
  // instr[9,5]   = Rn
  // instr[4,0]   = Rd

  // 32 bit operations must have N = 0 or else we have an UNALLOC
  // 64 bit operations must have N = 1 or else we have an UNALLOC
  u_int32_t size = uimm (instr, 31, 31);
  u_int32_t N = uimm (instr, 22, 22);
  if (size ^ N)
    return_UNALLOC;

  // 32 bit operations must have imms[5] = 0
  // or else we have an UNALLOC
  u_int32_t imms = uimm (instr, 15, 10);
  if (!size && uimm (imms, 5, 5))
    return_UNALLOC;

  // switch on combined size and op
  u_int32_t dispatch = uimm (instr, 31, 29);
  if (dispatch == 0)
    return do_EXTR_32 (); 

  if (dispatch == 4)
    return do_EXTR_64 (); 

  if (dispatch == 1) /* ???? */
    return_NYI;

  return_UNALLOC;
}

static StatusCode
dexDPImm (void)
{
  // u_int32_t group = dispatchGroup (instr);
  // assert  group == GROUP_DPIMM_1000 || grpoup == GROUP_DPIMM_1001
  // bits [25,23] of a DPImm are the secondary dispatch vector
  u_int32_t group2 = dispatchDPImm (instr);

  switch (group2)
    {
    case DPIMM_PCADR_000:
    case DPIMM_PCADR_001:
      return dexPCRelAddressing ();

    case DPIMM_ADDSUB_010:
    case DPIMM_ADDSUB_011:
      return dexAddSubtractImmediate ();

    case DPIMM_LOG_100:
      return dexLogicalImmediate ();

    case DPIMM_MOV_101:
      return dexMoveWideImmediate ();

    case DPIMM_BITF_110:
      return dexBitfieldImmediate ();

    case DPIMM_EXTR_111:
      return dexExtractImmediate ();
  }

  // should never reach here
  return report_and_die (UNIMPLEMENTED_INSTRUCTION);
}

static StatusCode
dexLoadUnscaledImmediate (void)
{
  // instr[29,24] == 111_00 AND instr[21] == 0 AND instr[11,10] == 00
  // instr[31,30] = size
  // instr[26] = V
  // instr[23,22] = opc
  // instr[20,12] = simm9
  // instr[9,5] = rn may be SP
  // unsigned rt = uimm (instr, 4, 0);
  u_int32_t V = uimm (instr, 26, 26);
  u_int32_t dispatch = ( (uimm (instr, 31, 30) << 2)
			| uimm (instr, 23, 22));
  int32_t imm = simm32 (instr, 20, 12);

  if (!V)
    {
      // GReg operations
      switch (dispatch)
	{
	case 0:	 return sturb (imm);
	case 1:	 return ldurb32 (imm);
	case 2:	 return ldursb64 (imm);
	case 3:	 return ldursb32 (imm);
	case 4:	 return sturh (imm);
	case 5:	 return ldurh32 (imm);
	case 6:	 return ldursh64 (imm);
	case 7:	 return ldursh32 (imm);
	case 8:	 return stur32 (imm);
	case 9:	 return ldur32 (imm);
	case 10: return ldursw (imm);
	case 12: return stur64 (imm);
	case 13: return ldur64 (imm);

	case 14:
	  // PRFUM NYI
	  return_NYI;

	default:
	case 11:
	case 15:
	  return_UNALLOC;
	}
    }

  // FReg operations
  switch (dispatch)
    {
    case 2:  return fsturq (imm);
    case 3:  return fldurq (imm);
    case 8:  return fsturs (imm);
    case 9:  return fldurs (imm);
    case 12: return fsturd (imm);
    case 13: return fldurd (imm);
      
    case 0: // STUR 8 bit FP
    case 1: // LDUR 8 bit FP
    case 4: // STUR 16 bit FP
    case 5: // LDUR 8 bit FP
      return_NYI;

    default:
    case 6:
    case 7:
    case 10:
    case 11:
    case 14:
    case 15:
      return_UNALLOC;
    }
}

////  n.b. a preliminary note regarding all the ldrs<x>32
//// instructions
//
// the signed value loaded by these instructions is cast to unsigned
// before being assigned to aarch64_get_reg_u64 (N) i.e. to the 64 bit element of the
// GReg union. this performs a 32 bit sign extension (as required) but
// avoids 64 bit sign extension, thus ensuring that the top half of
// the register word is zero. this is what the spec demands when a 32
// bit load occurs.

// 32 bit load sign-extended byte scaled unsigned 12 bit
static StatusCode
ldrsb32_abs (u_int32_t offset)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int rt = uimm (instr, 4, 0);

  // the target register may not be SP but the source may be
  // there is no scaling required for a byte load
  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK) + offset;
  return aarch64_set_reg_u64 (rt, NO_SP, (int64_t) aarch64_get_mem_s8 (address));
}

// 32 bit load sign-extended byte scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
ldrsb32_scale_ext (Scaling scaling, Extension extension)
{
  unsigned int rm = uimm (instr, 20, 16);
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int rt = uimm (instr, 4, 0);

  // rn may reference SP, rm and rt must reference ZR

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t displacement = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);

  // there is no scaling required for a byte load
  return aarch64_set_reg_u64 (rt, NO_SP, (int64_t) aarch64_get_mem_s8 (address + displacement));
}

// 32 bit load sign-extended byte unscaled signed 9 bit with pre- or post-writeback
static StatusCode
ldrsb32_wb (int32_t offset, WriteBack wb)
{
  unsigned int rn = uimm (instr, 9, 5);
  unsigned int rt = uimm (instr, 4, 0);
  
  if (rn == rt && wb != NoWriteBack)
    return_UNALLOC;

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb == Pre)
      address += offset;

  aarch64_set_reg_u64 (rt, NO_SP, (int64_t) aarch64_get_mem_s8 (address));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, NO_SP, address);

  return status;
}

// 8 bit store scaled
static StatusCode
fstrb_abs (u_int32_t offset)
{
  unsigned st = uimm (instr, 4, 0);
  unsigned rn = uimm (instr, 9, 5);

  return aarch64_set_mem_u8 (aarch64_get_reg_u64 (rn, SP_OK) + offset,
			     aarch64_get_vec_u8 (st, 0));
}

// 8 bit store scaled or unscaled zero- or sign-extended 8-bit register offset
static StatusCode
fstrb_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t  address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t    extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t  displacement = OPT_SCALE (extended, 32, scaling);

  return aarch64_set_mem_u8 (address + displacement, aarch64_get_vec_u8 (st, 0));
}

// 16 bit store scaled
static StatusCode
fstrh_abs (u_int32_t offset)
{
  unsigned st = uimm (instr, 4, 0);
  unsigned rn = uimm (instr, 9, 5);

  return aarch64_set_mem_u16 (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 16),
			      aarch64_get_vec_u16 (st, 0));
}

// 16 bit store scaled or unscaled zero- or sign-extended 16-bit register offset
static StatusCode
fstrh_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t  address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t    extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t  displacement = OPT_SCALE (extended, 32, scaling);

  return aarch64_set_mem_u16 (address + displacement, aarch64_get_vec_u16 (st, 0));
}

// 32 bit store scaled unsigned 12 bit
static StatusCode
fstrs_abs (u_int32_t offset)
{
  unsigned st = uimm (instr, 4, 0);
  unsigned rn = uimm (instr, 9, 5);

  return aarch64_set_mem_float (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 32),
				aarch64_get_FP_float (st));
}

// 32 bit store unscaled signed 9 bit with pre- or post-writeback
static StatusCode
fstrs_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_mem_float (address, aarch64_get_FP_float (st));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 32 bit store scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
fstrs_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t  address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t    extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t  displacement = OPT_SCALE (extended, 32, scaling);

  return aarch64_set_mem_float (address + displacement, aarch64_get_FP_float (st));
}

// 64 bit store scaled unsigned 12 bit
static StatusCode
fstrd_abs (u_int32_t offset)
{
  unsigned st = uimm (instr, 4, 0);
  unsigned rn = uimm (instr, 9, 5);

  return aarch64_set_mem_double (aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 64),
				 aarch64_get_FP_double (st));
}

// 64 bit store unscaled signed 9 bit with pre- or post-writeback
static StatusCode
fstrd_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  aarch64_set_mem_double (address, aarch64_get_FP_double (st));

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 64 bit store scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
fstrd_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t  address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t    extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t  displacement = OPT_SCALE (extended, 64, scaling);

  return aarch64_set_mem_double (address + displacement, aarch64_get_FP_double (st));
}

// 128 bit store scaled unsigned 12 bit
static StatusCode
fstrq_abs (u_int32_t offset)
{
  unsigned st = uimm (instr, 4, 0);
  unsigned rn = uimm (instr, 9, 5);

  FRegister a;
  aarch64_get_FP_long_double (st, & a);

  u_int64_t addr = aarch64_get_reg_u64 (rn, SP_OK) + SCALE (offset, 128);
  return aarch64_set_mem_long_double (addr, a);
}

// 128 bit store unscaled signed 9 bit with pre- or post-writeback
static StatusCode
fstrq_wb (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t address = aarch64_get_reg_u64 (rn, SP_OK);

  if (wb != Post)
    address += offset;

  FRegister a;
  aarch64_get_FP_long_double (st, & a);
  aarch64_set_mem_long_double (address, a);

  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rn, SP_OK, address);

  return status;
}

// 128 bit store scaled or unscaled zero- or sign-extended 32-bit register offset
static StatusCode
fstrq_scale_ext (Scaling scaling, Extension extension)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned st = uimm (instr, 4, 0);

  u_int64_t  address = aarch64_get_reg_u64 (rn, SP_OK);
  int64_t    extended = extend (aarch64_get_reg_u32 (rm, NO_SP), extension);
  u_int64_t  displacement = OPT_SCALE (extended, 128, scaling);

  FRegister a;
  aarch64_get_FP_long_double (st, & a);
  return aarch64_set_mem_long_double (address + displacement, a);
}

static StatusCode
dexLoadImmediatePrePost (void)
{
  // instr[29,24] == 111_00 AND instr[21] == 0 AND instr[11,10] == 00
  // instr[31,30] = size
  // instr[26] = V
  // instr[23,22] = opc
  // instr[20,12] = simm9
  // instr[11] = wb : 0 ==> Post, 1 ==> Pre
  // instr[9,5] = rn may be SP
  // unsigned rt = uimm (instr, 4, 0);
  u_int32_t V = uimm (instr, 26, 26);
  u_int32_t dispatch = ( (uimm (instr, 31, 30) << 2)
			| uimm (instr, 23, 22));
  int32_t imm = simm32 (instr, 20, 12);
  WriteBack wb = writeback (instr, 11);

  status = STATUS_READY;
  if (!V)
    {
      // GReg operations
      switch (dispatch)
	{
	case 0:	 return strb_wb (imm, wb);
	case 1:	 return ldrb32_wb (imm, wb);
	case 2:	 return ldrsb_wb (imm, wb);
	case 3:	 return ldrsb32_wb (imm, wb);
	case 4:	 return strh_wb (imm, wb);
	case 5:	 return ldrh32_wb (imm, wb);
	case 6:	 return ldrsh64_wb (imm, wb);
	case 7:	 return ldrsh32_wb (imm, wb);
	case 8:	 return str32_wb (imm, wb);
	case 9:	 return ldr32_wb (imm, wb);
	case 10: return ldrsw_wb (imm, wb);
	case 12: return str_wb (imm, wb);
	case 13: return ldr_wb (imm, wb);

	default:
	case 11:
	case 14:
	case 15:
	  return_UNALLOC;
	}
    }

  // FReg operations
  switch (dispatch)
    {
    case 2:  return fstrq_wb (imm, wb);
    case 3:  return fldrq_wb (imm, wb);
    case 8:  return fstrs_wb (imm, wb);
    case 9:  return fldrs_wb (imm, wb);
    case 12: return fstrd_wb (imm, wb);
    case 13: return fldrd_wb (imm, wb);

    case 0:	  // STUR 8 bit FP
    case 1:	  // LDUR 8 bit FP
    case 4:	  // STUR 16 bit FP
    case 5:	  // LDUR 8 bit FP
      return_NYI;

    default:
    case 6:
    case 7:
    case 10:
    case 11:
    case 14:
    case 15:
      return_UNALLOC;
    }
}

static StatusCode
dexLoadRegisterOffset (void)
{
  // instr[31,30] = size
  // instr[29,27] = 111
  // instr[26]    = V
  // instr[25,24] = 00
  // instr[23,22] = opc
  // instr[21]    = 1
  // instr[20,16] = rm
  // instr[15,13] = option : 010 ==> UXTW, 011 ==> UXTX/LSL,
  //                         110 ==> SXTW, 111 ==> SXTX,
  //                         ow ==> RESERVED
  // instr[12]    = scaled
  // instr[11,10] = 10
  // instr[9,5]   = rn
  // instr[4,0]   = rt

  u_int32_t V = uimm (instr, 26,26);
  u_int32_t dispatch = ( (uimm (instr, 31, 30) << 2)
			| uimm (instr, 23, 22));
  Scaling scale = scaling (instr, 12);
  Extension extensionType = extension (instr, 13);

  // check for illegal extension types
  if (uimm (extensionType, 1, 1) == 0)
    return_UNALLOC;

  if (extensionType == UXTX || extensionType == SXTX)
    extensionType = NoExtension;

  if (!V)
    {
      // GReg operations
      switch (dispatch)
	{
	case 0:	 return strb_scale_ext (scale, extensionType);
	case 1:	 return ldrb32_scale_ext (scale, extensionType);
	case 2:	 return ldrsb_scale_ext (scale, extensionType);
	case 3:	 return ldrsb32_scale_ext (scale, extensionType);
	case 4:	 return strh_scale_ext (scale, extensionType);
	case 5:	 return ldrh32_scale_ext (scale, extensionType);
	case 6:	 return ldrsh_scale_ext (scale, extensionType);
	case 7:	 return ldrsh32_scale_ext (scale, extensionType);
	case 8:	 return str32_scale_ext (scale, extensionType);
	case 9:	 return ldr32_scale_ext (scale, extensionType);
	case 10: return ldrsw_scale_ext (scale, extensionType);
	case 12: return str_scale_ext (scale, extensionType);
	case 13: return ldr_scale_ext (scale, extensionType);
	case 14: return prfm_scale_ext (scale, extensionType);

	default:
	case 11:
	case 15:
	  return_UNALLOC;
	}
    }

  // FReg operations
  switch (dispatch)
    {
    case 1: // LDUR 8 bit FP
      return_NYI;
    case 3:  return fldrq_scale_ext (scale, extensionType);
    case 5: // LDUR 8 bit FP
      return_NYI;
    case 9:  return fldrs_scale_ext (scale, extensionType);
    case 13: return fldrd_scale_ext (scale, extensionType);

    case 0:  return fstrb_scale_ext (scale, extensionType);
    case 2:  return fstrq_scale_ext (scale, extensionType);
    case 4:  return fstrh_scale_ext (scale, extensionType);
    case 8:  return fstrs_scale_ext (scale, extensionType);
    case 12: return fstrd_scale_ext (scale, extensionType);
	  
    default:
    case 6:
    case 7:
    case 10:
    case 11:
    case 14:
    case 15:
      return_UNALLOC;
    }
}

static StatusCode
dexLoadUnsignedImmediate (void)
{
  // assert instr[29,24] == 111_01
  // instr[31,30] = size
  // instr[26] = V
  // instr[23,22] = opc
  // instr[21,10] = uimm12 : unsigned immediate offset
  // instr[9,5] = rn may be SP
  // unsigned rt = uimm (instr, 4, 0);
  u_int32_t V = uimm (instr, 26,26);
  u_int32_t dispatch = ( (uimm (instr, 31, 30) << 2)
			| uimm (instr, 23, 22));
  u_int32_t imm = uimm (instr, 21, 10);

  if (!V)
    {
      // GReg operations
      switch (dispatch)
	{
	case 0:  return strb_abs (imm);
	case 1:  return ldrb32_abs (imm);
	case 2:  return ldrsb_abs (imm);
	case 3:  return ldrsb32_abs (imm);
	case 4:  return strh_abs (imm);
	case 5:  return ldrh32_abs (imm);
	case 6:  return ldrsh_abs (imm);
	case 7:  return ldrsh32_abs (imm);
	case 8:  return str32_abs (imm);
	case 9:  return ldr32_abs (imm);
	case 10: return ldrsw_abs (imm);
	case 12: return str_abs (imm);
	case 13: return ldr_abs (imm);
	case 14: return prfm_abs (imm);

	default:
	case 11:
	case 15:
	  return_UNALLOC;
	}
    }

  // FReg operations
  switch (dispatch)
    {
    case 3:  return fldrq_abs (imm);
    case 9:  return fldrs_abs (imm);
    case 13: return fldrd_abs (imm);

    case 0:  return fstrb_abs (imm);
    case 2:  return fstrq_abs (imm);
    case 4:  return fstrh_abs (imm);
    case 8:  return fstrs_abs (imm);
    case 12: return fstrd_abs (imm);

    case 1: // LDR 8 bit FP
    case 5: // LDR 8 bit FP
      return_NYI;

    default:
    case 6:
    case 7:
    case 10:
    case 11:
    case 14:
    case 15:
      return_UNALLOC;
    }
}

static StatusCode
dexLoadExclusive (void)
{
  // assert instr[29:24] = 001000;
  // instr[31,30] = size
  // instr[23] = 0 if exclusive
  // instr[22] = L : 1 if load, 0 if store
  // instr[21] = 1 if pair
  // instr[20,16] = Rs
  // instr[15] = o0 : 1 if ordered
  // instr[14,10] = Rt2
  // instr[9,5] = Rn
  // instr[4.0] = Rt

  switch (uimm (instr, 22, 21))
    {
    case 2:   return ldxr ();
    case 0:   return stxr ();
    default:  return_NYI;
    }
}

static StatusCode
dexLoadOther (void)
{
  // instr[29,25] = 111_0
  // instr[24] == 0 ==> dispatch, 1 ==> ldst reg unsigned immediate
  // instr[21:11,10] is the secondary dispatch
  if (uimm (instr, 24, 24))
    return dexLoadUnsignedImmediate ();

  u_int32_t dispatch = ( (uimm (instr, 21, 21) << 2)
			| uimm (instr, 11, 10));
  switch (dispatch)
    {
    case 0:      return dexLoadUnscaledImmediate ();
    case 1:      return dexLoadImmediatePrePost ();
    case 3:      return dexLoadImmediatePrePost ();
    case 6:      return dexLoadRegisterOffset ();

    default:
    case 2:
    case 4:
    case 5:
    case 7:
      return_NYI;
    }
}

static StatusCode
store_pair_u32 (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  if ((rn == rd || rm == rd) && wb != NoWriteBack)
    return_UNALLOC; /* ??? */

  offset <<= 2;

  if (wb != Post)
    address += offset;

  aarch64_set_mem_u32 (address,     aarch64_get_reg_u32 (rm, NO_SP));
  aarch64_set_mem_u32 (address + 4, aarch64_get_reg_u32 (rn, NO_SP));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
store_pair_u64 (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  if ((rn == rd || rm == rd) && wb != NoWriteBack)
    return_UNALLOC; /* ??? */

  offset <<= 3;

  if (wb != Post)
    address += offset;

  aarch64_set_mem_u64 (address,     aarch64_get_reg_u64 (rm, SP_OK));
  aarch64_set_mem_u64 (address + 8, aarch64_get_reg_u64 (rn, SP_OK));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
load_pair_u32 (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  // treat this as unalloc to make sure we don't do it
  if (rn == rm)
    return_UNALLOC;

  offset <<= 2;

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rm, SP_OK, aarch64_get_mem_u32 (address));
  aarch64_set_reg_u64 (rn, SP_OK, aarch64_get_mem_u32 (address + 4));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
load_pair_s32 (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  // treat this as unalloc to make sure we don't do it
  if (rn == rm)
    return_UNALLOC;

  offset <<= 2;

  if (wb != Post)
    address += offset;

  aarch64_set_reg_s64 (rm, SP_OK, aarch64_get_mem_s32 (address));
  aarch64_set_reg_s64 (rn, SP_OK, aarch64_get_mem_s32 (address + 4));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
load_pair_u64 (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  // treat this as unalloc to make sure we don't do it
  if (rn == rm)
    return_UNALLOC;

  offset <<= 3;

  if (wb != Post)
    address += offset;

  aarch64_set_reg_u64 (rm, SP_OK, aarch64_get_mem_u64 (address));
  aarch64_set_reg_u64 (rn, SP_OK, aarch64_get_mem_u64 (address + 8));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
dex_load_store_pair_gr (void)
{
  // instr[31,30] = size (10=> 64-bit, 01=> signed 32-bit, 00=> 32-bit)
  // instr[29,25] = instruction encoding: 101_0
  // instr[26]    = V : 1 if fp 0 if gp  
  // instr[24,23] = addressing mode (10=> offset, 01=> post, 11=> pre)
  // instr[22]    = load/store (1=> load)
  // instr[21,15] = signed, scaled, offset
  // instr[14,10] = Rn
  // instr[ 9, 5] = Rd
  // instr[ 4, 0] = Rm

  u_int32_t dispatch = ((uimm (instr, 31, 30) << 3)
			| uimm (instr, 24, 22));
  int32_t offset = simm32 (instr, 21, 15);

  switch (dispatch)
    {
    case 2: return store_pair_u32 (offset, Post);
    case 3: return load_pair_u32  (offset, Post);
    case 4: return store_pair_u32 (offset, NoWriteBack);
    case 5: return load_pair_u32  (offset, NoWriteBack);
    case 6: return store_pair_u32 (offset, Pre);
    case 7: return load_pair_u32  (offset, Pre);

    case 11: return load_pair_s32  (offset, Post);
    case 13: return load_pair_s32  (offset, NoWriteBack);
    case 15: return load_pair_s32  (offset, Pre);

    case 18: return store_pair_u64 (offset, Post);
    case 19: return load_pair_u64  (offset, Post);
    case 20: return store_pair_u64 (offset, NoWriteBack);
    case 21: return load_pair_u64  (offset, NoWriteBack);
    case 22: return store_pair_u64 (offset, Pre);
    case 23: return load_pair_u64  (offset, Pre);

    default:
    case 0:
    case 1:
    case 8:
    case 9:
    case 10:
    case 12:
    case 14:
    case 16:
    case 17:
    case 24 ... 31:
      return_UNALLOC;
    }
}

static StatusCode
store_pair_float (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  offset <<= 2;

  if (wb != Post)
    address += offset;

  aarch64_set_mem_float (address,     aarch64_get_FP_float (rm));
  aarch64_set_mem_float (address + 4, aarch64_get_FP_float (rn));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
store_pair_double (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  offset <<= 3;

  if (wb != Post)
    address += offset;

  aarch64_set_mem_double (address,     aarch64_get_FP_double (rm));
  aarch64_set_mem_double (address + 8, aarch64_get_FP_double (rn));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
store_pair_long_double (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  offset <<= 4;

  if (wb != Post)
    address += offset;

  FRegister a;
  aarch64_get_FP_long_double (rm, & a);
  aarch64_set_mem_long_double (address, a);
  aarch64_get_FP_long_double (rn, & a);
  aarch64_set_mem_long_double (address + 16, a);
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
load_pair_float (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  if (rm == rn)
    return_UNALLOC;

  offset <<= 2;

  if (wb != Post)
    address += offset;

  aarch64_set_FP_float (rm, aarch64_get_mem_float (address));
  aarch64_set_FP_float (rn, aarch64_get_mem_float (address + 4));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
load_pair_double (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  if (rm == rn)
    return_UNALLOC;

  offset <<= 3;

  if (wb != Post)
    address += offset;

  aarch64_set_FP_double (rm, aarch64_get_mem_double (address));
  aarch64_set_FP_double (rn, aarch64_get_mem_double (address + 8));
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
load_pair_long_double (int32_t offset, WriteBack wb)
{
  unsigned rn = uimm (instr, 14, 10);
  unsigned rd = uimm (instr, 9, 5);
  unsigned rm = uimm (instr, 4, 0);
  u_int64_t address = aarch64_get_reg_u64 (rd, SP_OK);

  if (rm == rn)
    return_UNALLOC;

  offset <<= 4;

  if (wb != Post)
    address += offset;

  FRegister a;
  aarch64_get_mem_long_double (address, & a);
  aarch64_set_FP_long_double (rm, a);
  aarch64_get_mem_long_double (address + 16, & a);
  aarch64_set_FP_long_double (rn, a);
  
  if (wb == Post)
    address += offset;

  if (wb != NoWriteBack)
    aarch64_set_reg_u64 (rd, SP_OK, address);

  return status;
}

static StatusCode
dex_load_store_pair_fp (void)
{
  // instr[31,30] = size (10=> 128-bit, 01=> 64-bit, 00=> 32-bit)
  // instr[29,25] = instruction encoding 
  // instr[24,23] = addressing mode (10=> offset, 01=> post, 11=> pre)
  // instr[22]    = load/store (1=> load)
  // instr[21,15] = signed, scaled, offset
  // instr[14,10] = Rn
  // instr[ 9, 5] = Rd
  // instr[ 4, 0] = Rm

  u_int32_t dispatch = ((uimm (instr, 31, 30) << 3)
			| uimm (instr, 24, 22));
  int32_t offset = simm32 (instr, 21, 15);

  switch (dispatch)
    {
    case 2: return store_pair_float (offset, Post);
    case 3: return load_pair_float  (offset, Post);
    case 4: return store_pair_float (offset, NoWriteBack);
    case 5: return load_pair_float  (offset, NoWriteBack);
    case 6: return store_pair_float (offset, Pre);
    case 7: return load_pair_float  (offset, Pre);

    case 10: return store_pair_double (offset, Post);
    case 11: return load_pair_double  (offset, Post);
    case 12: return store_pair_double (offset, NoWriteBack);
    case 13: return load_pair_double  (offset, NoWriteBack);
    case 14: return store_pair_double (offset, Pre);
    case 15: return load_pair_double  (offset, Pre);
      
    case 18: return store_pair_long_double (offset, Post);
    case 19: return load_pair_long_double  (offset, Post);
    case 20: return store_pair_long_double (offset, NoWriteBack);
    case 21: return load_pair_long_double  (offset, NoWriteBack);
    case 22: return store_pair_long_double (offset, Pre);
    case 23: return load_pair_long_double  (offset, Pre);

    default:
    case 0:
    case 1:
    case 8:
    case 9:
    case 16:
    case 17:
    case 24 ... 31:
      return_UNALLOC;
    }
}

static inline unsigned
vec_reg (unsigned v, unsigned o)
{
  return (v + o) & 0x3F;
}

/* Load multiple N-element structures to N consecutive registers.  */
static StatusCode
vec_load (u_int64_t address, unsigned N)
{
  int      all  = uimm (instr, 30, 30);
  unsigned size = uimm (instr, 11, 10);
  unsigned vd   = uimm (instr, 4, 0);
  unsigned i;

  switch (size)
    {
    case 0: /* 8-bit operations.  */
      if (all)
	for (i = 0; i < (16 * N); i++)
	  aarch64_set_vec_u8 (vec_reg (vd, i >> 4), i & 15,
			      aarch64_get_mem_u8 (address + i));
      else
	for (i = 0; i < (8 * N); i++)
	  aarch64_set_vec_u8 (vec_reg (vd, i >> 3), i & 7,
			      aarch64_get_mem_u8 (address + i));
      return status;
		  
    case 1: /* 16-bit operations.  */
      if (all)
	for (i = 0; i < (8 * N); i++)
	  aarch64_set_vec_u16 (vec_reg (vd, i >> 3), i & 7,
			       aarch64_get_mem_u16 (address + i * 2));
      else
	for (i = 0; i < (4 * N); i++)
	  aarch64_set_vec_u16 (vec_reg (vd, i >> 2), i & 3,
			       aarch64_get_mem_u16 (address + i * 2));
      return status;
		  
    case 2: /* 32-bit operations.  */
      if (all)
	for (i = 0; i < (4 * N); i++)
	  aarch64_set_vec_u32 (vec_reg (vd, i >> 2), i & 3,
			       aarch64_get_mem_u32 (address + i * 4));
      else
	for (i = 0; i < (2 * N); i++)
	  aarch64_set_vec_u32 (vec_reg (vd, i >> 1), i & 1,
			       aarch64_get_mem_u32 (address + i * 4));
      return status;

    case 3: /* 64-bit operations.  */
      if (all)
	for (i = 0; i < (2 * N); i++)
	  aarch64_set_vec_u64 (vec_reg (vd, i >> 1), i & 1,
			       aarch64_get_mem_u64 (address + i * 8));
      else
	for (i = 0; i < N; i++)
	  aarch64_set_vec_u64 (vec_reg (vd, i), 0,
			       aarch64_get_mem_u64 (address + i * 8));
      return status;

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

/* LD4: load multiple 4-element to four consecutive registers.  */
static StatusCode
LD4 (u_int64_t address)
{
  return vec_load (address, 4);
  /* LD4 {Vd.16b...Vd+3.16b}, addr, #64 */
  /* LD4 {Vd.8b...Vd+3.8b}, addr, #32 */
  /* LD4 {Vd.8h...Vd+3.8h}, addr, #64 */
  /* LD4 {Vd.4h...Vd+3.4h}, addr, #32 */
  /* LD4 {Vd.4s...Vd+3.4s}, addr, #64 */
  /* LD4 {Vd.2s...Vd+3.2s}, addr, #32 */
  /* LD4 {Vd.2d...Vd+3.2d}, addr, #64 */
  /* LD4 {Vd.1d...Vd+3.1d}, addr, #32 */
}

/* LD3: load multiple 3-element structures to three consecutive registers.  */
static StatusCode
LD3 (u_int64_t address)
{
  return vec_load (address, 3);
  /* LD3 {Vd.16b...Vd+2.16b}, addr, #48 */
  /* LD3 {Vd.8b...Vd+2.8b}, addr, #24 */
  /* LD3 {Vd.8h...Vd+2.8h}, addr, #48 */
  /* LD3 {Vd.4h...Vd+2.4h}, addr, #24 */
  /* LD3 {Vd.4s...Vd+2.4s}, addr, #48 */
  /* LD3 {Vd.2s...Vd+2.2s}, addr, #24 */
  /* LD3 {Vd.2d...Vd+2.2d}, addr, #48 */
  /* LD3 {Vd.1d...Vd+2.1d}, addr, #24 */
}

/* LD2: load multiple 2-element structures to two consecutive registers.  */
static StatusCode
LD2 (u_int64_t address)
{
  return vec_load (address, 2);
  /* LD2 {Vd.16b...Vd+1.16b}, addr, #32 */
  /* LD2 {Vd.8b...Vd+1.8b}, addr, #16 */
  /* LD2 {Vd.8h...Vd+1.8h}, addr, #32 */
  /* LD2 {Vd.4h...Vd+1.4h}, addr, #16 */
  /* LD2 {Vd.4s...Vd+1.4s}, addr, #48 */
  /* LD2 {Vd.2s...Vd+1.2s}, addr, #24 */
  /* LD2 {Vd.2d...Vd+1.2d}, addr, #48 */
  /* LD2 {Vd.1d...Vd+1.1d}, addr, #24 */
}

/* Load multiple 1-element structures into one register.  */
static StatusCode
LD1_1 (u_int64_t address)
{
  int      all  = uimm (instr, 30, 30);
  unsigned size = uimm (instr, 11, 10);
  unsigned vd   = uimm (instr, 4, 0);
  unsigned i;

  switch (size)
    {
    case 0:
      /* LD1 {Vd.16b}, addr, #16 */
      /* LD1 {Vd.8b}, addr, #8 */
      for (i = 0; i < (all ? 16 : 8); i++)
	aarch64_set_vec_u8 (vd, i, aarch64_get_mem_u8 (address + i));
      return status;

    case 1:
      /* LD1 {Vd.8h}, addr, #16 */
      /* LD1 {Vd.4h}, addr, #8 */
      for (i = 0; i < (all ? 8 : 4); i++)
	aarch64_set_vec_u16 (vd, i, aarch64_get_mem_u16 (address + i * 2));
      return status;

    case 2:
      /* LD1 {Vd.4s}, addr, #16 */
      /* LD1 {Vd.2s}, addr, #8 */
      for (i = 0; i < (all ? 4 : 2); i++)
	aarch64_set_vec_u32 (vd, i, aarch64_get_mem_u32 (address + i * 4));
      return status;

    case 3:
      /* LD1 {Vd.2d}, addr, #16 */
      /* LD1 {Vd.1d}, addr, #8 */
      for (i = 0; i < (all ? 2 : 1); i++)
	aarch64_set_vec_u64 (vd, i, aarch64_get_mem_u64 (address + i * 8));
      return status;

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

/* Load multiple 1-element structures into two registers.  */
static StatusCode
LD1_2 (u_int64_t address)
{
  /* FIXME: This algorithm is *exactly* the same as the LD2 version.
     So why have two different instructions ?  There must be something wrong somewhere.  */
  return vec_load (address, 2);
  /* LD1 {Vd.16b, Vd+1.16b}, addr, #32 */
  /* LD1 {Vd.8b, Vd+1.8b}, addr, #16 */
  /* LD1 {Vd.8h, Vd+1.8h}, addr, #32 */
  /* LD1 {Vd.4h, Vd+1.4h}, addr, #16 */
  /* LD1 {Vd.4s, Vd+1.4s}, addr, #32 */
  /* LD1 {Vd.2s, Vd+1.2s}, addr, #16 */
  /* LD1 {Vd.2d, Vd+1.2d}, addr, #32 */
  /* LD1 {Vd.1d, Vd+1.1d}, addr, #16 */
}

/* Load multiple 1-element structures into three registers.  */
static StatusCode
LD1_3 (u_int64_t address)
{
  /* FIXME: This algorithm is *exactly* the same as the LD3 version.
     So why have two different instructions ?  There must be something wrong somewhere.  */
  return vec_load (address, 3);
  /* LD1 {Vd.16b - Vd+2.16b}, addr, #48 */
  /* LD1 {Vd.8b  - Vd+2.8b}, addr, #24 */
  /* LD1 {Vd.8h - Vd+2.8h}, addr, #48 */
  /* LD1 {Vd.4h - Vd+2.4h}, addr, #24 */
  /* LD1 {Vd.4s - Vd+2.4s}, addr, #48 */
  /* LD1 {Vd.2s - Vd+2.2s}, addr, #24 */
  /* LD1 {Vd.2d - Vd+2.2d}, addr, #48 */
  /* LD1 {Vd.1d - Vd+2.1d}, addr, #24 */
}

/* Load multiple 1-element structures into four registers.  */
static StatusCode
LD1_4 (u_int64_t address)
{
  /* FIXME: This algorithm is *exactly* the same as the LD4 version.
     So why have two different instructions ?  There must be something wrong somewhere.  */
  return vec_load (address, 4);
  /* LD1 {Vd.16b - Vd+3.16b}, addr, #64 */
  /* LD1 {Vd.8b  - Vd+3.8b}, addr, #32 */
  /* LD1 {Vd.8h - Vd+3.8h}, addr, #64 */
  /* LD1 {Vd.4h - Vd+3.4h}, addr, #32 */
  /* LD1 {Vd.4s - Vd+3.4s}, addr, #64 */
  /* LD1 {Vd.2s - Vd+3.2s}, addr, #32 */
  /* LD1 {Vd.2d - Vd+3.2d}, addr, #48 */
  /* LD1 {Vd.1d - Vd+3.1d}, addr, #24 */
}

/* Store multiple N-element structures to N consecutive registers.  */
static StatusCode
vec_store (u_int64_t address, unsigned N)
{
  int      all  = uimm (instr, 30, 30);
  unsigned size = uimm (instr, 11, 10);
  unsigned vd   = uimm (instr, 4, 0);
  unsigned i;

  switch (size)
    {
    case 0: /* 8-bit operations.  */
      if (all)
	for (i = 0; i < (16 * N); i++)
	  aarch64_set_mem_u8 (address + i,
			      aarch64_get_vec_u8 (vec_reg (vd, i >> 4), i & 15));
      else
	for (i = 0; i < (8 * N); i++)
	  aarch64_set_mem_u8 (address + i,
			      aarch64_get_vec_u8 (vec_reg (vd, i >> 3), i & 7));
      return status;
		  
    case 1: /* 16-bit operations.  */
      if (all)
	for (i = 0; i < (8 * N); i++)
	  aarch64_set_mem_u16 (address + i * 2,
			       aarch64_get_vec_u16 (vec_reg (vd, i >> 3), i & 7));
      else
	for (i = 0; i < (4 * N); i++)
	  aarch64_set_mem_u16 (address + i * 2,
			       aarch64_get_vec_u16 (vec_reg (vd, i >> 2), i & 3));
      return status;
		  
    case 2: /* 32-bit operations.  */
      if (all)
	for (i = 0; i < (4 * N); i++)
	  aarch64_set_mem_u32 (address + i * 4,
			       aarch64_get_vec_u32 (vec_reg (vd, i >> 2), i & 3));
      else
	for (i = 0; i < (2 * N); i++)
	  aarch64_set_mem_u32 (address + i * 4,
			       aarch64_get_vec_u32 (vec_reg (vd, i >> 1), i & 1));
      return status;

    case 3: /* 64-bit operations.  */
      if (all)
	for (i = 0; i < (2 * N); i++)
	  aarch64_set_mem_u64 (address + i * 8,
			       aarch64_get_vec_u64 (vec_reg (vd, i >> 1), i & 1));
      else
	for (i = 0; i < N; i++)
	  aarch64_set_mem_u64 (address + i * 8,
			       aarch64_get_vec_u64 (vec_reg (vd, i), 0));
      return status;

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

/* Store multiple 4-element structure to four consecutive registers.  */
static StatusCode
ST4 (u_int64_t address)
{
  return vec_store (address, 4);
}

/* Store multiple 3-element structures to three consecutive registers.  */
static StatusCode
ST3 (u_int64_t address)
{
  return vec_store (address, 3);
}

/* Store multiple 2-element structures to two consecutive registers.  */
static StatusCode
ST2 (u_int64_t address)
{
  return vec_store (address, 2);
}

/* Store multiple 1-element structures into one register.  */
static StatusCode
ST1_1 (u_int64_t address)
{
  int      all  = uimm (instr, 30, 30);
  unsigned size = uimm (instr, 11, 10);
  unsigned vd   = uimm (instr, 4, 0);
  unsigned i;

  switch (size)
    {
    case 0:
      for (i = 0; i < (all ? 16 : 8); i++)
	aarch64_set_mem_u8 (address + i, aarch64_get_vec_u8 (vd, i));
      return status;

    case 1:
      for (i = 0; i < (all ? 8 : 4); i++)
	aarch64_set_mem_u16 (address + i * 2, aarch64_get_vec_u16 (vd, i));
      return status;

    case 2:
      for (i = 0; i < (all ? 4 : 2); i++)
	aarch64_set_mem_u32 (address + i * 4, aarch64_get_vec_u32 (vd, i));
      return status;

    case 3:
      for (i = 0; i < (all ? 2 : 1); i++)
	aarch64_set_mem_u64 (address + i * 8, aarch64_get_vec_u64 (vd, i));
      return status;

    default:
      return report_and_die (INTERNAL_ERROR);
    }
}

/* Store multiple 1-element structures into two registers.  */
static StatusCode
ST1_2 (u_int64_t address)
{
  /* FIXME: This algorithm is *exactly* the same as the ST2 version.
     So why have two different instructions ?  There must be something wrong somewhere.  */
  return vec_store (address, 2);
}

/* Store multiple 1-element structures into three registers.  */
static StatusCode
ST1_3 (u_int64_t address)
{
  /* FIXME: This algorithm is *exactly* the same as the ST3 version.
     So why have two different instructions ?  There must be something wrong somewhere.  */
  return vec_store (address, 3);
}

/* Store multiple 1-element structures into four registers.  */
static StatusCode
ST1_4 (u_int64_t address)
{
  /* FIXME: This algorithm is *exactly* the same as the ST4 version.
     So why have two different instructions ?  There must be something wrong somewhere.  */
  return vec_store (address, 4);
}

static StatusCode
do_vec_LDnR (u_int64_t address)
{
  // instr[31]    = 0
  // instr[30]    = element selector 0=>half, 1=>all elements
  // instr[29,24] = 00 1101
  // instr[23]    = 0=>simple, 1=>post
  // instr[22]    = 1
  // instr[21]    = width: LD1R-or-LD3R (0) / LD2R-or-LD4R (1)
  // instr[20,16] = 0 0000 (simple), Vinc (reg-post-inc, no SP), 11111 (immediate post inc)
  // instr[15,14] = 11
  // instr[13]    = width: LD1R-or-LD2R (0) / LD3R-or-LD4R (1)
  // instr[12]    = 0
  // instr[11,10] = element size 00=> byte(b), 01=> half(h), 10=> word(s), 11=> double(d)
  // instr[9,5]   = address
  // instr[4,0]   = Vd

  NYI_assert (29, 24, 0x0D);
  NYI_assert (22, 22, 1);
  NYI_assert (15, 14, 3);
  NYI_assert (12, 12, 0);

  unsigned full = uimm (instr, 30, 30);
  unsigned vd = uimm (instr, 4, 0);
  unsigned size = uimm (instr, 11, 10);
  int i;

  switch ((uimm (instr, 13, 13) << 1) | uimm (instr, 21, 21))
    {
    case 0: // LD1R
      switch (size)
	{
	case 0:
	  {
	    u_int8_t val = aarch64_get_mem_u8 (address);
	    for (i = 0; i < (full ? 16 : 8); i++)
	      aarch64_set_vec_u8 (vd, i, val);
	    break;
	  }

	case 1:
	  {
	    u_int16_t val = aarch64_get_mem_u16 (address);
	    for (i = 0; i < (full ? 8 : 4); i++)
	      aarch64_set_vec_u16 (vd, i, val);
	    break;
	  }

	case 2:
	  {
	    u_int32_t val = aarch64_get_mem_u32 (address);
	    for (i = 0; i < (full ? 4 : 2); i++)
	      aarch64_set_vec_u32 (vd, i, val);
	    break;
	  }

	case 3:
	  {
	    u_int64_t val = aarch64_get_mem_u64 (address);
	    for (i = 0; i < (full ? 2 : 1); i++)
	      aarch64_set_vec_u64 (vd, i, val);
	    break;
	  }

	default:
	  return_UNALLOC;
	}
      break;
      
    case 1: // LD2R
      switch (size)
	{
	case 0:
	  {
	    u_int8_t val1 = aarch64_get_mem_u8 (address);
	    u_int8_t val2 = aarch64_get_mem_u8 (address + 1);

	    for (i = 0; i < (full ? 16 : 8); i++)
	      {
		aarch64_set_vec_u8 (vd, 0, val1);
		aarch64_set_vec_u8 (vd + 1, 0, val2);
	      }
	    break;
	  }

	case 1:
	  {
	    u_int16_t val1 = aarch64_get_mem_u16 (address);
	    u_int16_t val2 = aarch64_get_mem_u16 (address + 2);

	    for (i = 0; i < (full ? 8 : 4); i++)
	      {
		aarch64_set_vec_u16 (vd, 0, val1);
		aarch64_set_vec_u16 (vd + 1, 0, val2);
	      }
	    break;
	  }

	case 2:
	  {
	    u_int32_t val1 = aarch64_get_mem_u32 (address);
	    u_int32_t val2 = aarch64_get_mem_u32 (address + 4);

	    for (i = 0; i < (full ? 4 : 2); i++)
	      {
		aarch64_set_vec_u32 (vd, 0, val1);
		aarch64_set_vec_u32 (vd + 1, 0, val2);
	      }
	    break;
	  }

	case 3:
	  {
	    u_int64_t val1 = aarch64_get_mem_u64 (address);
	    u_int64_t val2 = aarch64_get_mem_u64 (address + 8);

	    for (i = 0; i < (full ? 2 : 1); i++)
	      {
		aarch64_set_vec_u64 (vd, 0, val1);
		aarch64_set_vec_u64 (vd + 1, 0, val2);
	      }
	    break;
	  }

	default:
	  return_UNALLOC;
	}
      break;

    case 2: // LD3R
      switch (size)
	{
	case 0:
	  {
	    u_int8_t val1 = aarch64_get_mem_u8 (address);
	    u_int8_t val2 = aarch64_get_mem_u8 (address + 1);
	    u_int8_t val3 = aarch64_get_mem_u8 (address + 2);

	    for (i = 0; i < (full ? 16 : 8); i++)
	      {
		aarch64_set_vec_u8 (vd, 0, val1);
		aarch64_set_vec_u8 (vd + 1, 0, val2);
		aarch64_set_vec_u8 (vd + 2, 0, val3);
	      }
	  }
	  break;

	case 1:
	  {
	    u_int32_t val1 = aarch64_get_mem_u16 (address);
	    u_int32_t val2 = aarch64_get_mem_u16 (address + 2);
	    u_int32_t val3 = aarch64_get_mem_u16 (address + 4);

	    for (i = 0; i < (full ? 8 : 4); i++)
	      {
		aarch64_set_vec_u16 (vd, 0, val1);
		aarch64_set_vec_u16 (vd + 1, 0, val2);
		aarch64_set_vec_u16 (vd + 2, 0, val3);
	      }
	  }
	  break;

	case 2:
	  {
	    u_int32_t val1 = aarch64_get_mem_u32 (address);
	    u_int32_t val2 = aarch64_get_mem_u32 (address + 4);
	    u_int32_t val3 = aarch64_get_mem_u32 (address + 8);

	    for (i = 0; i < (full ? 4 : 2); i++)
	      {
		aarch64_set_vec_u32 (vd, 0, val1);
		aarch64_set_vec_u32 (vd + 1, 0, val2);
		aarch64_set_vec_u32 (vd + 2, 0, val3);
	      }
	  }
	  break;

	case 3:
	  {
	    u_int64_t val1 = aarch64_get_mem_u64 (address);
	    u_int64_t val2 = aarch64_get_mem_u64 (address + 8);
	    u_int64_t val3 = aarch64_get_mem_u64 (address + 16);

	    for (i = 0; i < (full ? 2 : 1); i++)
	      {
		aarch64_set_vec_u64 (vd, 0, val1);
		aarch64_set_vec_u64 (vd + 1, 0, val2);
		aarch64_set_vec_u64 (vd + 2, 0, val3);
	      }
	  }
	  break;

	default:
	  return_UNALLOC;
	}
      break;
      
    case 3: // LD4R
      switch (size)
	{
	case 0:
	  {
	    u_int8_t val1 = aarch64_get_mem_u8 (address);
	    u_int8_t val2 = aarch64_get_mem_u8 (address + 1);
	    u_int8_t val3 = aarch64_get_mem_u8 (address + 2);
	    u_int8_t val4 = aarch64_get_mem_u8 (address + 3);

	    for (i = 0; i < (full ? 16 : 8); i++)
	      {
		aarch64_set_vec_u8 (vd, 0, val1);
		aarch64_set_vec_u8 (vd + 1, 0, val2);
		aarch64_set_vec_u8 (vd + 2, 0, val3);
		aarch64_set_vec_u8 (vd + 3, 0, val4);
	      }
	  }
	  break;

	case 1:
	  {
	    u_int32_t val1 = aarch64_get_mem_u16 (address);
	    u_int32_t val2 = aarch64_get_mem_u16 (address + 2);
	    u_int32_t val3 = aarch64_get_mem_u16 (address + 4);
	    u_int32_t val4 = aarch64_get_mem_u16 (address + 6);

	    for (i = 0; i < (full ? 8 : 4); i++)
	      {
		aarch64_set_vec_u16 (vd, 0, val1);
		aarch64_set_vec_u16 (vd + 1, 0, val2);
		aarch64_set_vec_u16 (vd + 2, 0, val3);
		aarch64_set_vec_u16 (vd + 3, 0, val4);
	      }
	  }
	  break;

	case 2:
	  {
	    u_int32_t val1 = aarch64_get_mem_u32 (address);
	    u_int32_t val2 = aarch64_get_mem_u32 (address + 4);
	    u_int32_t val3 = aarch64_get_mem_u32 (address + 8);
	    u_int32_t val4 = aarch64_get_mem_u32 (address + 12);

	    for (i = 0; i < (full ? 4 : 2); i++)
	      {
		aarch64_set_vec_u32 (vd, 0, val1);
		aarch64_set_vec_u32 (vd + 1, 0, val2);
		aarch64_set_vec_u32 (vd + 2, 0, val3);
		aarch64_set_vec_u32 (vd + 3, 0, val4);
	      }
	  }
	  break;

	case 3:
	  {
	    u_int64_t val1 = aarch64_get_mem_u64 (address);
	    u_int64_t val2 = aarch64_get_mem_u64 (address + 8);
	    u_int64_t val3 = aarch64_get_mem_u64 (address + 16);
	    u_int64_t val4 = aarch64_get_mem_u64 (address + 24);

	    for (i = 0; i < (full ? 2 : 1); i++)
	      {
		aarch64_set_vec_u64 (vd, 0, val1);
		aarch64_set_vec_u64 (vd + 1, 0, val2);
		aarch64_set_vec_u64 (vd + 2, 0, val3);
		aarch64_set_vec_u64 (vd + 3, 0, val4);
	      }
	  }
	  break;

	default:
	  return_UNALLOC;
	}
      break;

    default:
      return_UNALLOC;
    }

  return status;
}

static StatusCode
do_vec_load_store (void)
{
  // {LD|ST}<N>   {Vd..Vd+N}, vaddr
  //
  // instr[31]    = 0
  // instr[30]    = element selector 0=>half, 1=>all elements
  // instr[29,25] = 00110
  // instr[24]    = ?
  // instr[23]    = 0=>simple, 1=>post
  // instr[22]    = 0=>store, 1=>load
  // instr[21]    = 0 (LDn) / small(0)-large(1) selector (LDnR)
  // instr[20,16] = 00000 (simple), Vinc (reg-post-inc, no SP), 11111 (immediate post inc)
  // instr[15,12] = elements and destinations.  eg for load:
  //                 0000=>LD4 => load multiple 4-element to four consecutive registers
  //                 0100=>LD3 => load multiple 3-element to three consecutive registers
  //                 1000=>LD2 => load multiple 2-element to two consecutive registers
  //                 0010=>LD1 => load multiple 1-element to four consecutive registers
  //                 0110=>LD1 => load multiple 1-element to three consecutive registers
  //                 1010=>LD1 => load multiple 1-element to two consecutive registers
  //                 0111=>LD1 => load multiple 1-element to one register
  //                 1100=>LDR1,LDR2
  //                 1110=>LDR3,LDR4
  // instr[11,10] = element size 00=> byte(b), 01=> half(h), 10=> word(s), 11=> double(d)
  // instr[9,5]   = Vn, can be SP
  // instr[4,0]   = Vd

  if (uimm (instr, 31, 31) != 0
      || uimm (instr, 29, 25) != 0x06)
    return_NYI;

  int type = uimm (instr, 15, 12);
  if (type != 0xE && type != 0xE && uimm (instr, 21, 21) != 0)
    return_NYI;
  
  int post = uimm (instr, 23, 23);
  int load = uimm (instr, 22, 22);
  unsigned vn = uimm (instr, 9, 5);
  u_int64_t address = aarch64_get_reg_u64 (vn, SP_OK);

  if (post)
    {
      unsigned vm = uimm (instr, 20, 16);

      if (vm == R31)
	{
	  unsigned sizeof_operation;

	  switch (type)
	    {
	    case 0: sizeof_operation = 32; break;
	    case 4: sizeof_operation = 24; break;
	    case 8: sizeof_operation = 16; break;

	    case 0xC:
	      sizeof_operation = uimm (instr, 21, 21) ? 2 : 1;
	      sizeof_operation <<= uimm (instr, 11, 10);
	      break;

	    case 0xE:
	      sizeof_operation = uimm (instr, 21, 21) ? 8 : 4;
	      sizeof_operation <<= uimm (instr, 11, 10);
	      break;
	      
	    case 2:
	    case 6:
	    case 10:
	    case 7:
	      sizeof_operation = 2 << uimm (instr, 11, 10);
	      break;

	    default:
	      return_UNALLOC;
	    }

	  if (uimm (instr, 30, 30))
	    sizeof_operation *= 2;

	  aarch64_set_reg_u64 (vn, SP_OK, address + sizeof_operation);
	}
      else
	aarch64_set_reg_u64 (vn, SP_OK, address + aarch64_get_reg_u64 (vm, NO_SP));
    }
  else
    {
      NYI_assert (20, 16, 0);
    }
    
  if (load)
    {
      switch (type)
	{
	case 0: return  LD4 (address);
	case 4:  return LD3 (address);	  
	case 8:  return LD2 (address);
	case 2:  return LD1_4 (address);
	case 6:  return LD1_3 (address);
	case 10: return LD1_2 (address);
	case 7:  return LD1_1 (address);

	case 0xE:
	case 0xC: return do_vec_LDnR (address);

	default:
	  return_NYI;
	}
    }

  /* Stores.  */
  switch (type)
    {
    case 0:  return ST4 (address);
    case 4:  return ST3 (address);	  
    case 8:  return ST2 (address);
    case 2:  return ST1_4 (address);
    case 6:  return ST1_3 (address);
    case 10: return ST1_2 (address);
    case 7:  return ST1_1 (address);
    default:
      return_NYI;
    }
}

static StatusCode
dexLdSt (void)
{
  // u_int32_t group = dispatchGroup (instr);
  // assert  group == GROUP_LDST_0100 || group == GROUP_LDST_0110 ||
  //         group == GROUP_LDST_1100 || group == GROUP_LDST_1110
  // bits [29,28:26] of a LS are the secondary dispatch vector
  u_int32_t group2 = dispatchLS (instr);

  switch (group2)
    {
    case LS_EXCL_000:
      return dexLoadExclusive ();

    case LS_LIT_010:
    case LS_LIT_011:
      return dexLoadLiteral ();

    case LS_OTHER_110:
    case LS_OTHER_111:
      return dexLoadOther ();

    case LS_ADVSIMD_001:
      return do_vec_load_store ();

    case LS_PAIR_100:
      return dex_load_store_pair_gr ();

    case LS_PAIR_101:
      return dex_load_store_pair_fp ();

    default:
      /* Should never reach here.  */
      return report_and_die (UNIMPLEMENTED_INSTRUCTION);
    }
}

// specific decode and execute for group Data Processing Register

static StatusCode
dexLogicalShiftedRegister (void)
{
  // assert instr[28:24] = 01010
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30,29:21] = op,N : 000 ==> AND, 001 ==> BIC,
  //                           010 ==> ORR, 011 ==> ORN
  //                           100 ==> EOR, 101 ==> EON,
  //                           110 ==> ANDS, 111 ==> BICS
  // instr[23,22] = shift : 0 ==> LSL, 1 ==> LSR, 2 ==> ASR, 3 ==> ROR
  // unsigned rm = uimm (instr, 20, 16);
  // instr[15,10] = count : must be 0xxxxx for 32 bit
  // instr[9,5] = Rn
  // instr[4,0] = Rd

  u_int32_t size = uimm (instr, 31, 31);

  // 32 bit operations must have count[5] = 0
  // or else we have an UNALLOC
  u_int32_t count = uimm (instr, 15, 10);
  if (!size && uimm (count, 5, 5))
    return_UNALLOC;

  Shift shiftType = shift (instr, 22);

  // dispatch on size:op:N i.e instr[31,29:21]
  u_int32_t dispatch = ( (uimm (instr, 31, 29) << 1)
			| uimm (instr, 21, 21));

  switch (dispatch)
    {
    case 0: return and32_shift (shiftType, count);
    case 1: return bic32_shift (shiftType, count);
    case 2: return orr32_shift (shiftType, count);
    case 3: return orn32_shift (shiftType, count);
    case 4: return eor32_shift (shiftType, count);
    case 5: return eon32_shift (shiftType, count);
    case 6: return ands32_shift (shiftType, count);
    case 7: return bics32_shift (shiftType, count);
    case 8: return and64_shift (shiftType, count);
    case 9: return bic64_shift (shiftType, count);
    case 10:return orr64_shift (shiftType, count);
    case 11:return orn64_shift (shiftType, count);
    case 12:return eor64_shift (shiftType, count);
    case 13:return eon64_shift (shiftType, count);
    case 14:return ands64_shift (shiftType, count);
    case 15:return bics64_shift (shiftType, count);
    default: return_UNALLOC;
    }
}

// 32 bit conditional select
static StatusCode
csel32 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u32 (rn, NO_SP)
			      : aarch64_get_reg_u32 (rm, NO_SP));
}

// 64 bit conditional select
static StatusCode
csel64 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u64 (rn, NO_SP)
			      : aarch64_get_reg_u64 (rm, NO_SP));
}

// 32 bit conditional increment
static StatusCode
csinc32 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u32 (rn, NO_SP)
			      : aarch64_get_reg_u32 (rm, NO_SP) + 1);
}

// 64 bit conditional increment
static StatusCode
csinc64 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u64 (rn, NO_SP)
			      : aarch64_get_reg_u64 (rm, NO_SP) + 1);
}

// 32 bit conditional invert
static StatusCode
csinv32 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u32 (rn, NO_SP)
			      : ~ aarch64_get_reg_u32 (rm, NO_SP));
}

// 64 bit conditional invert
static StatusCode
csinv64 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u64 (rn, NO_SP)
			      : ~ aarch64_get_reg_u64 (rm, NO_SP));
}

// 32 bit conditional negate
static StatusCode
csneg32 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u32 (rn, NO_SP)
			      : - aarch64_get_reg_u32 (rm, NO_SP));
}

// 64 bit conditional negate
static StatusCode
csneg64 (CondCode cc)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      testConditionCode (cc)
			      ? aarch64_get_reg_u64 (rn, NO_SP)
			      : - aarch64_get_reg_u64 (rm, NO_SP));
}

static StatusCode
dexCondSelect (void)
{
  // assert instr[28,21] = 11011011
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[30:11,10] = op : 000 ==> CSEL, 001 ==> CSINC,
  //                        100 ==> CSINV, 101 ==> CSNEG,
  //                        _1_ ==> UNALLOC
  // instr[29] = S : 0 ==> ok, 1 ==> UNALLOC
  // instr[15,12] = cond
  // instr[29] = S : 0 ==> ok, 1 ==> UNALLOC

  u_int32_t S = uimm (instr, 29, 29);
  if (S == 1)
    return_UNALLOC;

  u_int32_t op2 = uimm (instr, 11, 10);
  if (op2 & 0x2)
    return_UNALLOC;

  CondCode cc = condcode (instr, 12);
  u_int32_t dispatch = ((uimm (instr, 31, 30) << 1) | op2);

  switch (dispatch)
    {
    case 0: return csel32 (cc);
    case 1: return csinc32 (cc);
    case 2: return csinv32 (cc);
    case 3: return csneg32 (cc);
    case 4: return csel64 (cc);
    case 5: return csinc64 (cc);
    case 6: return csinv64 (cc);
    case 7: return csneg64 (cc);
    default: return_UNALLOC;
    }
}

// some helpers for counting leading 1 or 0 bits
//
// hmm, is there not some intel asm we can call to do this?

// counts the number of leading bits which are the same
// in a 32 bit value in the range 1 to 32
static u_int32_t
leading32 (u_int32_t value)
{
  int32_t mask= 0xffff0000;
  u_int32_t count= 16; // counts number of bits set in mask
  u_int32_t lo = 1;    // lower bound for number of sign bits
  u_int32_t hi = 32;   // upper bound for number of sign bits

  while (lo + 1 < hi)
    {
      int32_t test = (value & mask);

      if (test == 0 || test == mask)
	{
	  lo = count;
	  count = (lo + hi) / 2;
	  mask >>= (count - lo);
	}
      else
	{
	  hi = count;
	  count = (lo + hi) / 2;
	  mask <<= hi - count;
	}
    }
  
  if (lo != hi)
    {
      mask >>= 1;

      int32_t test = (value & mask);

      if (test == 0 || test == mask)
	count = hi;
      else
	count = lo;
    }

  return count;
}

// counts the number of leading bits which are the same
// in a 64 bit value in the range 1 to 64
static u_int64_t
leading64 (u_int64_t value)
{
  int64_t mask= 0xffffffff00000000LL;
  u_int64_t count = 32; // counts number of bits set in mask
  u_int64_t lo = 1;     // lower bound for number of sign bits
  u_int64_t hi = 64;    // upper bound for number of sign bits

  while (lo + 1 < hi)
    {
      int64_t test = (value & mask);

      if (test == 0 || test == mask)
	{
	  lo = count;
	  count = (lo + hi) / 2;
	  mask >>= (count - lo);
	}
      else
	{
	  hi = count;
	  count = (lo + hi) / 2;
	  mask <<= hi - count;
	}
    }

  if (lo != hi)
    {
      mask >>= 1;
      int64_t test = (value & mask);

      if (test == 0 || test == mask)
	count = hi;
      else
	count = lo;
    }

  return count;
}

// bit operations
//
// n.b register args may not be SP.

// 32 bit count leading sign bits
static StatusCode
cls32 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  // n.b. the result needs to exclude the leading bit
  return aarch64_set_reg_u64 (rd, NO_SP, leading32 (aarch64_get_reg_u32 (rn, NO_SP)) - 1);
}

// 64 bit count leading sign bits
static StatusCode
cls64 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  // n.b. the result needs to exclude the leading bit
  return aarch64_set_reg_u64 (rd, NO_SP, leading64 (aarch64_get_reg_u64 (rn, NO_SP)) - 1);
}

// 32 bit count leading zero bits
static StatusCode
clz32 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int32_t value = aarch64_get_reg_u32 (rn, NO_SP);

  // if the sign (top) bit is set then the count is 0
  if (pick32 (value, 31, 31))
    return aarch64_set_reg_u64 (rd, NO_SP, 0L);

  return aarch64_set_reg_u64 (rd, NO_SP, leading32 (value));
}

// 64 bit count leading zero bits
static StatusCode
clz64 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int64_t value = aarch64_get_reg_u64 (rn, NO_SP);

  // if the sign (top) bit is set then the count is 0
  if (pick64 (value, 63, 63)) 
    return aarch64_set_reg_u64 (rd, NO_SP, 0L);

  return aarch64_set_reg_u64 (rd, NO_SP, leading64 (value));
}

// 32 bit reverse bits
static StatusCode
rbit32 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int32_t value = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t result = 0;
  int i;
  
  for (i = 0; i < 32; i++)
    {
      result <<= 1;
      result |= (value & 1);
      value >>= 1;
    }
  return aarch64_set_reg_u64 (rd, NO_SP, result);
}

// 64 bit reverse bits
static StatusCode
rbit64 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int64_t value = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t result = 0;
  int i;
  
  for (i = 0; i < 64; i++)
    {
      result <<= 1;
      result |= (value & 1L);
      value >>= 1;
    }
  return aarch64_set_reg_u64 (rd, NO_SP, result);
}

// 32 bit reverse bytes
static StatusCode
rev32 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int32_t value = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t result = 0;
  int i;

  for (i = 0; i < 4; i++)
    {
      result <<= 8;
      result |= (value & 0xff);
      value >>= 8;
    }
  return aarch64_set_reg_u64 (rd, NO_SP, result);
}

// 64 bit reverse bytes
static StatusCode
rev64 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int64_t value = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t result = 0;
  int i;
    
  for (i = 0; i < 8; i++)
    {
      result <<= 8;
      result |= (value & 0xffULL);
      value >>= 8;
    }
  return aarch64_set_reg_u64 (rd, NO_SP, result);
}

// 32 bit reverse shorts
// n.b.this reverses the order of the bytes in each half word
static StatusCode
revh32 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int32_t value = aarch64_get_reg_u32 (rn, NO_SP);
  u_int32_t result = 0;
  int i;

  for (i = 0; i < 2; i++)
    {
      result <<= 8;
      result |= (value & 0x00ff00ff);
      value >>= 8;
    }
  return aarch64_set_reg_u64 (rd, NO_SP, result);
}

// 64 bit reverse shorts
// n.b.this reverses the order of the bytes in each half word
static StatusCode
revh64 (void)
{
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  u_int64_t value = aarch64_get_reg_u64 (rn, NO_SP);
  u_int64_t result = 0;
  int i;

  for (i = 0; i < 2; i++)
    {
      result <<= 8;
      result |= (value & 0x00ff00ff00ff00ffULL);
      value >>= 8;
    }
  return aarch64_set_reg_u64 (rd, NO_SP, result);
}

static StatusCode
dexDataProc1Source (void)
{
  // assert instr[30] == 1 AND instr[28,21] == 111010110
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[29] = S : 0 ==> ok, 1 ==> UNALLOC
  // instr[20,16] = opcode2 : 00000 ==> ok, ow ==> UNALLOC
  // instr[15,10] = opcode : 000000 ==> RBIT, 000001 ==> REV16,
  //                         000010 ==> REV, 000011 ==> UNALLOC
  //                         000100 ==> CLZ, 000101 ==> CLS
  //                         ow ==> UNALLOC
  // instr[9,5] = rn : may not be SP
  // instr[4,0] = rd : may not be SP
  u_int32_t S = uimm (instr, 29, 29);
  if (S == 1)
    return_UNALLOC;

  u_int32_t opcode2 = uimm (instr, 20, 16);
  if (opcode2 != 0)
    return_UNALLOC;

  u_int32_t opcode = uimm (instr, 15, 10);
  if (opcode & 0x38)
    return_UNALLOC;

  u_int32_t dispatch = ((uimm (instr, 31, 31) << 3) | opcode);

  switch (dispatch)
    {
    case 0: return rbit32 ();
    case 1: return revh32 ();
    case 2: return rev32 ();
    case 4: return clz32 ();
    case 5: return cls32 ();
    case 8: return rbit64 ();
    case 9: return revh64 ();
    case 10:return rev32 ();
    case 11:return rev64 ();
    case 12:return clz64 ();
    case 13:return cls64 ();
    default: return_UNALLOC;
    }
}

// variable shift
// shifts by count supplied in register
//
// n.b register args may not be SP.
//
// these all use the shifted auxiliary function for
// simplicity and clarity. writing the actual shift
// inline would avoid a branch and so be faster but
// would also necessitate getting signs right

// 32 bit arithmetic shift right
static StatusCode
asrv32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted32 (aarch64_get_reg_u32 (rn, NO_SP), ASR,
					 (aarch64_get_reg_u32 (rm, NO_SP) & 0x1f)));
}

// 64 bit arithmetic shift right
static StatusCode
asrv64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted64 (aarch64_get_reg_u64 (rn, NO_SP), ASR,
					 (aarch64_get_reg_u64 (rm, NO_SP) & 0x3f)));
}

// 32 bit logical shift left
static StatusCode
lslv32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted32 (aarch64_get_reg_u32 (rn, NO_SP), LSL,
					 (aarch64_get_reg_u32 (rm, NO_SP) & 0x1f)));
}

// 64 bit arithmetic shift left
static StatusCode
lslv64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted64 (aarch64_get_reg_u64 (rn, NO_SP), LSL,
					 (aarch64_get_reg_u64 (rm, NO_SP) & 0x3f)));
}

// 32 bit logical shift right
static StatusCode
lsrv32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted32 (aarch64_get_reg_u32 (rn, NO_SP), LSR,
					 (aarch64_get_reg_u32 (rm, NO_SP) & 0x1f)));
}

// 64 bit logical shift right
static StatusCode
lsrv64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted64 (aarch64_get_reg_u64 (rn, NO_SP), LSR,
					 (aarch64_get_reg_u64 (rm, NO_SP) & 0x3f)));
}

// 32 bit rotate right
static StatusCode
rorv32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted32 (aarch64_get_reg_u32 (rn, NO_SP), ROR,
					 (aarch64_get_reg_u32 (rm, NO_SP) & 0x1f)));
}

// 64 bit rotate right
static StatusCode
rorv64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      shifted64 (aarch64_get_reg_u64 (rn, NO_SP), ROR,
					 (aarch64_get_reg_u64 (rm, NO_SP) & 0x3f)));
}


// divide

// 32 bit signed divide
static StatusCode
sdiv32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);
  // n.b. the pseudo-code does the divide using 64 bit data
  // TODO : check that this rounds towards zero as required
  int64_t dividend = aarch64_get_reg_s32 (rn, NO_SP);
  int64_t divisor = aarch64_get_reg_s32 (rm, NO_SP);

  return aarch64_set_reg_s64 (rd, NO_SP,
			      divisor ? ((int32_t) (dividend / divisor)) : 0);
}

// 64 bit signed divide
static StatusCode
sdiv64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // TODO : check that this rounds towards zero as required
  int64_t divisor = aarch64_get_reg_s64 (rm, NO_SP);

  return aarch64_set_reg_s64 (rd, NO_SP,
			      divisor ? (aarch64_get_reg_s64 (rn, NO_SP) / divisor) : 0);
}

// 32 bit unsigned divide
static StatusCode
udiv32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // n.b. the pseudo-code does the divide using 64 bit data
  u_int64_t dividend = aarch64_get_reg_u32 (rn, NO_SP);
  u_int64_t divisor  = aarch64_get_reg_u32 (rm, NO_SP);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      divisor ? (u_int32_t) (dividend / divisor) : 0);
}

// 64 bit unsigned divide
static StatusCode
udiv64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // TODO : check that this rounds towards zero as required
  u_int64_t divisor = aarch64_get_reg_u64 (rm, NO_SP);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      divisor ? (aarch64_get_reg_u64 (rn, NO_SP) / divisor) : 0);
}

static StatusCode
dexDataProc2Source (void)
{
  // assert instr[30] == 0 AND instr[28,21] == 11010110
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit
  // instr[29] = S : 0 ==> ok, 1 ==> UNALLOC
  // instr[15,10] = opcode : 000010 ==> UDIV, 000011 ==> SDIV,
  //                         001000 ==> LSLV, 001001 ==> LSRV
  //                         001010 ==> ASRV, 001011 ==> RORV
  //                         ow ==> UNALLOC

  u_int32_t S = uimm (instr, 29, 29);
  if (S == 1)
    return_UNALLOC;

  u_int32_t opcode = uimm (instr, 15, 10);

  if (opcode & 0x34)
    return_UNALLOC;

  u_int32_t dispatch = (  (uimm (instr, 31, 31) << 3)
			| (uimm (opcode, 3, 3) << 2)
			|  uimm (opcode, 1, 0));
  switch (dispatch)
    {
    case 2: return udiv32 ();
    case 3: return sdiv32 ();
    case 4: return lslv32 ();
    case 5: return lsrv32 ();
    case 6: return asrv32 ();
    case 7: return rorv32 ();
    case 10:return udiv64 ();
    case 11:return sdiv64 ();
    case 12:return lslv64 ();
    case 13:return lsrv64 ();
    case 14:return asrv64 ();
    case 15:return rorv64 ();
    default: return_UNALLOC;
    }
}


// multiply

// 32 bit multiply and add
static StatusCode
madd32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (ra, NO_SP)
			      + aarch64_get_reg_u32 (rn, NO_SP)
			      * aarch64_get_reg_u32 (rm, NO_SP));
}

// 64 bit multiply and add
static StatusCode
madd64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (ra, NO_SP)
			      + aarch64_get_reg_u64 (rn, NO_SP)
			      * aarch64_get_reg_u64 (rm, NO_SP));
}

// 32 bit multiply and sub
static StatusCode
msub32 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u32 (ra, NO_SP)
			      - aarch64_get_reg_u32 (rn, NO_SP)
			      * aarch64_get_reg_u32 (rm, NO_SP));
}

// 64 bit multiply and sub
static StatusCode
msub64 (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (ra, NO_SP)
			      - aarch64_get_reg_u64 (rn, NO_SP)
			      * aarch64_get_reg_u64 (rm, NO_SP));
}

// signed multiply add long -- source, source2 : 32 bit, source3 : 64 bit
static StatusCode
smaddl (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);  
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // n.b. we need to multiply the signed 32 bit values in rn, rm to
  // obtain a 64 bit product
  return aarch64_set_reg_s64 (rd, NO_SP,
			      aarch64_get_reg_s64 (ra, NO_SP)
			      + ((int64_t) aarch64_get_reg_s32 (rn, NO_SP))
			      * ((int64_t) aarch64_get_reg_s32 (rm, NO_SP)));
}

// signed multiply sub long -- source, source2 : 32 bit, source3 : 64 bit
static StatusCode
smsubl (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // n.b. we need to multiply the signed 32 bit values in rn, rm to
  // obtain a 64 bit product
  return aarch64_set_reg_s64 (rd, NO_SP,
			      aarch64_get_reg_s64 (ra, NO_SP)
			      - ((int64_t) aarch64_get_reg_s32 (rn, NO_SP))
			      * ((int64_t) aarch64_get_reg_s32 (rm, NO_SP)));
}

//// Integer Multiply/Divide

// first some macros and a helper function
//
// macros to test or access elements of 64 bit words

// mask used to access lo 32 bits of 64 bit unsigned int
#define LOW_WORD_MASK ((1ULL << 32) - 1)
// return the lo 32 bit word of a 64 bit unsigned int as a 64 bit unsigned int
#define lowWordToU64(_value_u64) ((_value_u64) & LOW_WORD_MASK)
// return the hi 32 bit word of a 64 bit unsigned int as a 64 bit unsigned int
#define highWordToU64(_value_u64) ((_value_u64) >> 32)

// offset of sign bit in 64 bit signed integger
#define SIGN_SHIFT_U64 63
// the sign bit itself -- also identifies the minimum negative int value
#define SIGN_BIT_U64 (1UL << SIGN_SHIFT_U64)
// return true if a 64 bit signed int presented as an unsigned int is the most negative value
#define isMinimumU64(_value_u64) ((_value_u64) == SIGN_BIT_U64)
// return true (non-zero) if a 64 bit signed int presented as an unsigned int has its sign bit set ow false
#define isSignSetU64(_value_u64) ((_value_u64) & SIGN_BIT_U64)
// return 1L or -1L according to whether a 64 bit signed int presented as an unsigned int has its sign bit set or not
#define signOfU64(_value_u64) (1L + (((value_u64) >> SIGN_SHIFT_U64) * -2L)
// clear the sign bit of a 64 bit signed int presented as an unsigned int
#define clearSignU64(_value_u64) ((_value_u64) &= ~SIGN_BIT_U64)
//
// multiply two 64 bit ints and return
// the hi 64 bits of the 128 bit product
//
static u_int64_t
mul64hi (u_int64_t value1, u_int64_t value2)
{
  u_int64_t value1_lo = lowWordToU64 (value1);
  u_int64_t value1_hi = highWordToU64 (value1) ;
  u_int64_t value2_lo = lowWordToU64 (value2);
  u_int64_t value2_hi = highWordToU64 (value2);

  // cross-multiply and collect results

  u_int64_t xproductlo = value1_lo * value2_lo;
  u_int64_t xproductmid1 = value1_lo * value2_hi;
  u_int64_t xproductmid2 = value1_hi * value2_lo;
  u_int64_t xproducthi = value1_hi * value2_hi;
  u_int64_t carry = 0;
  // start accumulating 64 bit results
  // drop bottom half of lowest cross-product
  u_int64_t resultmid = xproductlo >> 32;
  // add in middle products
  resultmid = resultmid + xproductmid1;

  // check for overflow
  if (resultmid < xproductmid1)
    {
      // carry over 1 into top cross-product
      carry++;
    }

  u_int64_t resultmid1  = resultmid + xproductmid2;

  // check for overflow
  if (resultmid1 < xproductmid2)
    {
      // carry over 1 into top cross-product
      carry++;
    }

  // drop lowest 32 bits of middle cross-product
  u_int64_t result = resultmid1 >> 32;

  // add top cross-product plus and any carry
  result += xproducthi + carry;

  return result;
}

// signed multiply high, source, source2 : 64 bit, dest <-- high 64-bit of result
static StatusCode
smulh (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  GReg ra = greg (instr, 10);
  if (ra != R31)
    return_UNALLOC;

  // convert to unsigned and use the unsigned mul64hi routine
  // the fix the sign up afterwards

  int64_t value1 = aarch64_get_reg_u64 (rn, NO_SP);
  int64_t value2 = aarch64_get_reg_u64 (rm, NO_SP);
  u_int64_t uvalue1;
  u_int64_t uvalue2;
  int64_t signum = 1;

  if (value1 < 0)
    {
      signum *= -1L;
      uvalue1 = -value1;
    }
  else
    {
      uvalue1 = value1;
    }

  if (value2 < 0)
    {
      signum *= -1L;
      uvalue2 = -value2;
    }
  else
    {
      uvalue2 = value2;
    }

  u_int64_t uresult = mul64hi (uvalue1, uvalue2);
  int64_t result = uresult;
  result *= signum;

  return aarch64_set_reg_s64 (rd, NO_SP, result);
}

// unsigned multiply add long -- source, source2 : 32 bit, source3 : 64 bit
static StatusCode
umaddl (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // n.b. we need to multiply the signed 32 bit values in rn, rm to
  // obtain a 64 bit product
  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (ra, NO_SP)
			      + ((u_int64_t) aarch64_get_reg_u32 (rn, NO_SP))
			      * ((u_int64_t) aarch64_get_reg_u32 (rm, NO_SP)));
}

// unsigned multiply sub long -- source, source2 : 32 bit, source3 : 64 bit
static StatusCode
umsubl (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned ra = uimm (instr, 14, 10);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  // n.b. we need to multiply the signed 32 bit values in rn, rm to
  // obtain a 64 bit product
  return aarch64_set_reg_u64 (rd, NO_SP,
			      aarch64_get_reg_u64 (ra, NO_SP)
			      - ((u_int64_t) aarch64_get_reg_u32 (rn, NO_SP))
			      * ((u_int64_t) aarch64_get_reg_u32 (rm, NO_SP)));
}

// unsigned multiply high, source, source2 : 64 bit, dest <-- high 64-bit of result
static StatusCode
umulh (void)
{
  unsigned rm = uimm (instr, 20, 16);
  unsigned rn = uimm (instr, 9, 5);
  unsigned rd = uimm (instr, 4, 0);

  GReg ra = greg (instr, 10);
  if (ra != R31)
    return_UNALLOC;
  
  return aarch64_set_reg_u64 (rd, NO_SP,
			      mul64hi (aarch64_get_reg_u64 (rn, NO_SP),
				       aarch64_get_reg_u64 (rm, NO_SP)));
}

static StatusCode
dexDataProc3Source (void)
{
  // assert instr[28,24] == 11011
  // instr[31] = size : 0 ==> 32 bit, 1 ==> 64 bit (for rd at least)
  // instr[30,29] = op54 : 00 ==> ok, ow ==> UNALLOC
  // instr[23,21] = op31 : 111 ==> UNALLOC, o2 ==> ok
  // instr[15] = o0 : 0/1 ==> ok
  // instr[23,21:15] ==> op : 0000 ==> MADD, 0001 ==> MSUB,     (32/64 bit)
  //                          0010 ==> SMADDL, 0011 ==> SMSUBL, (64 bit only)
  //                          0100 ==> SMULH,                   (64 bit only)
  //                          1010 ==> UMADDL, 1011 ==> UNSUBL, (64 bit only)
  //                          1100 ==> UMULH                    (64 bit only)
  //                          ow ==> UNALLOC

  u_int32_t size = uimm (instr, 31, 31);
  u_int32_t op54 = uimm (instr, 30, 29);
  if (op54 != 0)
    return_UNALLOC;

  u_int32_t op31 = uimm (instr, 23, 21);
  u_int32_t o0 = uimm (instr, 15, 15);

  if (size == 0)
    {
      if (op31 != 0)
	return_UNALLOC;

      if (o0 == 0)
	return madd32 ();

      return msub32 ();
    }

  u_int32_t dispatch = (op31 << 1) | o0;

  switch (dispatch)
    {
    case 0:  return madd64 ();
    case 1:  return msub64 ();
    case 2:  return smaddl ();
    case 3:  return smsubl ();
    case 4:  return smulh ();
    case 10: return umaddl ();
    case 11: return umsubl ();
    case 12: return umulh ();
    default:
      return_UNALLOC;
    }
}

static StatusCode
dexDPReg (void)
{
  // u_int32_t group = dispatchGroup (instr);
  // assert  group == GROUP_DPREG_0101 || group == GROUP_DPREG_1101 
  // bits [28:24:21] of a DPReg are the secondary dispatch vector
  u_int32_t group2 = dispatchDPReg (instr);

  switch (group2)
    {
    case DPREG_LOG_000:
    case DPREG_LOG_001:
      return dexLogicalShiftedRegister ();

    case DPREG_ADDSHF_010:
      return dexAddSubtractShiftedRegister ();

    case DPREG_ADDEXT_011:
      return dexAddSubtractExtendedRegister ();

    case DPREG_ADDCOND_100:
      {
	// this set bundles a variety of different operations
	//
	// check for
	//
	// 1) add/sub w carry
	u_int32_t mask1 = 0x1FE00000U;
	u_int32_t val1  = 0x1A000000U;
	// 2) cond compare register/immediate
	u_int32_t mask2 = 0x1FE00000U;
	u_int32_t val2  = 0x1A400000U;
	// 3) cond select
	u_int32_t mask3 = 0x1FE00000U;
	u_int32_t val3  = 0x1A800000U;
	// 4) data proc 1/2 source
	u_int32_t mask4 = 0x1FE00000U;
	u_int32_t val4  = 0x1AC00000U;

	if ((instr & mask1) == val1)
	  return dexAddSubtractWithCarry ();

	if ((instr & mask2) == val2)
	  return CondCompare ();

	if ((instr & mask3) == val3)
	  return dexCondSelect ();

	if ((instr & mask4) == val4)
	  {
	    // bit 30 is clear for data proc 2 source
	    // and set for data proc 1 source
	    if (instr  & (1U << 30))
	      return dexDataProc1Source ();
	    return dexDataProc2Source ();
	  }

	// Should not reach here.  */
	return_NYI;
      }

    case DPREG_3SRC_110:
      return dexDataProc3Source ();

    case DPREG_UNALLOC_101:
      return_UNALLOC;

    case DPREG_3SRC_111:
      return dexDataProc3Source ();
    }

  /* Should never reach here.  */
  return report_and_die (UNIMPLEMENTED_INSTRUCTION);
}

// unconditional Branch immediate
// offset is a PC-relative byte offset in the range +/- 128MiB
//
// the offset is assumed to be raw from the decode i.e. the
// simulator is expected to scale them from word offsets to byte
// offsets???

// unconditional branch
static StatusCode
buc (int32_t offset)
{
  return aarch64_set_next_PC_by_offset (offset);
}

static unsigned stack_depth = 0;

// unconditional branch and link -- writes return PC to LR
static StatusCode
bl (int32_t offset)
{
  aarch64_save_LR ();
  aarch64_set_next_PC_by_offset (offset);
  if (trace & TRACE_FUNCTIONS)
    {
      ++ stack_depth;
      fprintf (stderr, " %*scall %" PRINT_64 "x [%s]"
	       " [args: %" PRINT_64"x %" PRINT_64 "x %" PRINT_64 "x]\n",
	       stack_depth, " ", aarch64_get_next_PC (),
	       aarch64_print_func (aarch64_get_next_PC ()),
	       aarch64_get_reg_u64 (0, NO_SP),
	       aarch64_get_reg_u64 (1, NO_SP),
	       aarch64_get_reg_u64 (2, NO_SP)
	       );
    }
  return status;
}

// unconditional Branch register
// branch/return address is in source register

// unconditional branch
static StatusCode
br (void)
{
  unsigned rn = uimm (instr, 9, 5);
  return aarch64_set_next_PC (aarch64_get_reg_u64 (rn, NO_SP));
}

// unconditional branch and link -- writes return PC to LR
static StatusCode
blr (void)
{
  unsigned rn = uimm (instr, 9, 5);
  // the pseudo code in the spec says we update LR before fetching
  // the value from the rn
  aarch64_save_LR ();
  aarch64_set_next_PC (aarch64_get_reg_u64 (rn, NO_SP));
  if (trace & TRACE_FUNCTIONS)
    {
      ++ stack_depth;
      fprintf (stderr, " %*scall %" PRINT_64 "x [%s]"
	       " [args: %" PRINT_64"x %" PRINT_64 "x %" PRINT_64 "x]\n",
	       stack_depth, " ", aarch64_get_next_PC (),
	       aarch64_print_func (aarch64_get_next_PC ()),
	       aarch64_get_reg_u64 (0, NO_SP),
	       aarch64_get_reg_u64 (1, NO_SP),
	       aarch64_get_reg_u64 (2, NO_SP)
	       );
    }
  return status;
}

// return -- assembler will default source to LR this is functionally
// equivalent to br but, presumably, unlike br it side effects the
// branch predictor.
static StatusCode
ret (void)
{
  unsigned rn = uimm (instr, 9, 5);
  aarch64_set_next_PC (aarch64_get_reg_u64 (rn, NO_SP));

  if (trace & TRACE_FUNCTIONS)
    {
      fprintf (stderr, " %*sreturn [result: %" PRINT_64 "x]\n",
	       stack_depth, " ", aarch64_get_reg_u64 (0, NO_SP));
      -- stack_depth;
    }

  return status;
}

// move gp register to system register

void
msr (void)
{
  // currently we only get here if the system register is FPSR
  // instr[4,0] = Rt
  // only the bottom 32 bits are relevant
  aarch64_set_FPSR (aarch64_get_reg_u32 (uimm (instr, 4, 0), NO_SP));
}

// move system register to gp register

void
mrs (void)
{
  // currently we only get here if the system register is FPSR
  // instr[4,0] = Rt
  // only the bottom 32 bits are relevant
  aarch64_set_reg_u64 (uimm (instr, 4, 0), NO_SP, aarch64_get_FPSR ());
}

// nop -- we implement this and call it from the decode in case we
// want to intercept it later

static StatusCode
nop (void)
{
  return status;
}

// data synchronization barrier

static StatusCode
dsb (void)
{
  return status;
}

// data memory barrier

static StatusCode
dmb (void)
{
  return status;
}

// instruction synchronization barrier

static StatusCode
isb (void)
{
  return status;
}

// TODO specific decode and execute for group Branch/Exception/System

static StatusCode
dexBranchImmediate (void)
{
  // assert instr[30,26] == 00101
  // instr[31] ==> 0 == B, 1 == BL
  // instr[25,0] == imm26 branch offset counted in words

  u_int32_t top = uimm (instr, 31, 31);
  // we have a 26 byte signed word offset which we need to pass to the
  // execute routine as a signed byte offset
  int32_t offset = simm32 (instr, 25, 0) << 2;

  if (top)
    return bl (offset);

  return buc (offset);
}

//// Control Flow

// conditional branch
//
// offset is a PC-relative byte offset in the range +/- 1MiB pos is
// a bit position in the range 0 .. 63
//
// cc is a CondCode enum value as pulled out of the decode
//
// n.b. any offset register (source) can only be Xn or Wn.

static StatusCode
bcc (int32_t offset, CondCode cc)
{
  // the test returns TRUE if CC is met
  if (testConditionCode (cc))
    aarch64_set_next_PC_by_offset (offset);

  return status;
}

// 32 bit branch on register non-zero
static StatusCode
cbnz32 (int32_t offset)
{
  unsigned rt = uimm (instr, 4, 0);

  if (aarch64_get_reg_u32 (rt, NO_SP) != 0)
    aarch64_set_next_PC_by_offset (offset);

  return status;
}

// 64 bit branch on register zero
static StatusCode
cbnz (int32_t offset)
{
  unsigned rt = uimm (instr, 4, 0);

  if (aarch64_get_reg_u64 (rt, NO_SP) != 0)
    aarch64_set_next_PC_by_offset (offset);

  return status;
}

// 32 bit branch on register non-zero
static StatusCode
cbz32 (int32_t offset)
{
  unsigned rt = uimm (instr, 4, 0);

  if (aarch64_get_reg_u32 (rt, NO_SP) == 0)
    aarch64_set_next_PC_by_offset (offset);

  return status;
}

// 64 bit branch on register zero
static StatusCode
cbz (int32_t offset)
{
  unsigned rt = uimm (instr, 4, 0);

  if (aarch64_get_reg_u64 (rt, NO_SP) == 0)
    aarch64_set_next_PC_by_offset (offset);

  return status;
}

// branch on register bit test non-zero -- one size fits all
static StatusCode
tbnz (u_int32_t  pos, int32_t offset)
{
  unsigned rt = uimm (instr, 4, 0);

  if (aarch64_get_reg_u64 (rt, NO_SP) & (1 << pos))
    aarch64_set_next_PC_by_offset (offset);

  return status;
}

// branch on register bit test zero -- one size fits all
static StatusCode
tbz (u_int32_t  pos, int32_t offset)
{
  unsigned rt = uimm (instr, 4, 0);

  if (!(aarch64_get_reg_u64 (rt, NO_SP) & (1 << pos)))
    aarch64_set_next_PC_by_offset (offset);

  return status;
}

static StatusCode
dexCompareBranchImmediate (void)
{
  // assert instr[30,25] == 011010
  // instr[31] = size : 0 ==> 32, 1 ==> 64
  // instr[24] = op : 0 ==> CBZ, 1 ==> CBNZ
  // instr[23,5] = simm19 branch offset counted in words
  // unsigned rt = uimm (instr, 4, 0);

  u_int32_t size = uimm (instr, 31, 31);
  u_int32_t op   = uimm (instr, 24, 24);
  int32_t offset = simm32 (instr, 23, 5) << 2;

  if (size == 0)
    {
      if (op == 0)
	cbz32 (offset);
      else
	cbnz32 (offset);
    }
  else
    {
      if (op == 0)
	cbz (offset);
      else
	cbnz (offset);
    }

  return status;
}

static StatusCode
dexTestBranchImmediate (void)
{
  // assert instr[30,25] = 011011
  // instr[31] = b5 : bit 5 of test bit idx
  // instr[23,19] = b40 : bits 4 to 0 of test bit idx
  // instr[24] = op : 0 ==> TBZ, 1 == TBNZ
  // instr[18,5] = simm14 : signed offset counted in words
  // unsigned rt = uimm (instr, 4, 0);
  u_int32_t pos = ((uimm (instr, 31, 31) << 4) | uimm (instr, 23,19));
  int32_t offset = simm32 (instr, 18, 5) << 2;

  if (uimm (instr, 24, 24) == 0)
    return tbz (pos, offset);

  return tbnz (pos, offset);
}

static StatusCode
dexCondBranchImmediate (void)
{
  // assert instr[31,25] = 0101010
  // instr[24:4] = o1:o0 : 00 ==> B.cond, ow ==> UNALLOC
  // instr[23,5] = simm19 : signed offset counted in words
  // instr[3,0] = cond
  u_int32_t op = ((uimm (instr, 24, 24) << 1) | uimm (instr, 4, 4));

  if (op != 0)
    return_UNALLOC;

  int32_t offset = simm32 (instr, 23, 5) << 2;
  CondCode cc = condcode (instr, 0);

  return bcc (offset, cc);
}

static StatusCode
dexBranchRegister (void)
{
  // assert instr[31,25] = 1101011
  // instr[24,21] = op : 0 ==> BR, 1 ==> BLR, 2 ==> RET, 3 ==> ERET, 4 ==> DRPS
  // instr[20,16] = op2 : must be 11111
  // instr[15,10] = op3 : must be 000000
  // instr[4,0] = op2 : must be 11111
  u_int32_t op = uimm (instr, 24, 21);
  u_int32_t op2 = uimm (instr, 20, 16);
  u_int32_t op3 = uimm (instr, 15, 10);
  u_int32_t op4 = uimm (instr, 4, 0);

  if (op2 != 0x1F || op3 != 0 || op4 != 0)
    return_UNALLOC;
  
  if (op == 0)
    br ();

  else if (op == 1)
    blr ();

  else if (op == 2)
    ret ();

  else
    {
      // ERET and DRPS accept 0b11111 for rn = instr[4,0]
      // anything else is unallocated
      u_int32_t rn = greg (instr, 0);

      if (rn != 0x1f)
	return_UNALLOC;

      if (op == 4 || op == 5)
	return_NYI;

      return_UNALLOC;
    }
  return status;
}

static StatusCode
handle_halt (u_int32_t val)
{
  u_int64_t result = 0;

  if (val != 0xf000)
    {
      if (trace & TRACE_MISC)
	fprintf (stderr, " HLT [0x%x]\n", val);
      return STATUS_HALT;
    }

  /* We have encountered an Angel SVC call.  See if we can process it.  */
  switch (aarch64_get_reg_u32 (0, NO_SP))
    {
    case AngelSVC_Reason_HeapInfo:
      {
	/* Get the pointer  */
	u_int64_t ptr = aarch64_get_reg_u64 (1, SP_OK);
	ptr = aarch64_get_mem_u64 (ptr);

	/* Get the values.  */
	u_int64_t stack_top = aarch64_get_stack_start ();
	u_int64_t heap_base = aarch64_get_heap_start ();

	/* Fill in the memory block.  */
	aarch64_set_mem_u64 (ptr +  0, heap_base); /* Start addr of heap.  */
	aarch64_set_mem_u64 (ptr +  8, stack_top); /* End addr of heap.  */
	aarch64_set_mem_u64 (ptr + 16, heap_base); /* Lowest stack addr.  */
	aarch64_set_mem_u64 (ptr + 24, stack_top); /* Initial stack addr.  */

	if (trace & TRACE_MISC)
	  fprintf (stderr, " AngelSVC: Get Heap Info\n");
      }
      break;

    case AngelSVC_Reason_Open:
      {
	/* Get the pointer  */
	// u_int64_t ptr = aarch64_get_reg_u64 (1, SP_OK);
	/* FIXME: For now we just assume that we will only be asked to open the
	   standard file descriptors.  */
	static int fd = 0;
	result = fd ++;

	if (trace & TRACE_MISC)
	  fprintf (stderr, " AngelSVC: Open file %d\n", fd - 1);
      }
      break;
      
    case AngelSVC_Reason_Close:
      {
	u_int64_t fh = aarch64_get_reg_u64 (1, SP_OK);
	if (trace & TRACE_MISC)
	  fprintf (stderr, " AngelSVC: Close file %d\n", (int) fh);
	result = 0;
      }
      break;
      

    case AngelSVC_Reason_Errno:
      result = status;
      if (trace & TRACE_MISC)
	fprintf (stderr, " AngelSVC: Get Errno\n");
      break;

    case AngelSVC_Reason_Clock:
      result = 
#ifdef CLOCKS_PER_SEC
	(CLOCKS_PER_SEC >= 100)
	? (clock () / (CLOCKS_PER_SEC / 100))
	: ((clock () * 100) / CLOCKS_PER_SEC)
#else
	/* Presume unix... clock() returns microseconds.  */
	(clock () / 10000)
#endif
	;
      if (trace & TRACE_MISC)
	fprintf (stderr, " AngelSVC: Get Clock\n");
      break;

    case AngelSVC_Reason_GetCmdLine:
      {
	/* Get the pointer  */
	u_int64_t ptr = aarch64_get_reg_u64 (1, SP_OK);
	ptr = aarch64_get_mem_u64 (ptr);

	// FIXME: No command line for now.
	aarch64_set_mem_u64 (ptr, 0);
	if (trace & TRACE_MISC)
	  fprintf (stderr, " AngelSVC: Get Command Line\n");
      }
      break;
      
    case AngelSVC_Reason_IsTTY:
      result = 1;
      if (trace & TRACE_MISC)
	fprintf (stderr, " AngelSVC: IsTTY ?\n");
      break;

    case AngelSVC_Reason_Write:
      {
	/* Get the pointer  */
	u_int64_t ptr = aarch64_get_reg_u64 (1, SP_OK);
	/* Get the write control block.  */
	u_int64_t fd  = aarch64_get_mem_u64 (ptr);
	u_int64_t buf = aarch64_get_mem_u64 (ptr + 8);
	u_int64_t len = aarch64_get_mem_u64 (ptr + 16);

	if (trace & TRACE_MISC)
	  fprintf (stderr, "write of %" PRINT_64 "x bytes from %" PRINT_64 "x on descriptor %" PRINT_64 "x\n",
		   len, buf, fd);

	if (len > 1280)
	  {
	    if (trace & TRACE_MISC)
	      fprintf (stderr, " AngelSVC: Write: Suspiciously long write: %ld\n", (long) len);
	    return STATUS_HALT;
	  }
	else if (fd == 1)
	  {
	    printf ("%.*s", (int) len, aarch64_get_mem_ptr (buf));
	    if (trace || disas)
	      // So that the output stays in sync with trace output.
	      fflush (stdout);
	  }
	else if (fd == 2)
	  {
	    if (trace)
	      fprintf (stderr, "\n%.*s\n", (int) len, aarch64_get_mem_ptr (buf));
	    else
	      fprintf (stderr, "%.*s", (int) len, aarch64_get_mem_ptr (buf));
	  }
	else
	  {
	    if (trace & TRACE_MISC)
	      fprintf (stderr, " AngelSVC: Write: Unexpected file handle: %d\n", (int) fd);
	    return STATUS_HALT;
	  }
      }
      break;

    case AngelSVC_Reason_ReportException:
      {
	/* Get the pointer  */
	u_int64_t ptr = aarch64_get_reg_u64 (1, SP_OK);
	//ptr = aarch64_get_mem_u64 (ptr);
	u_int64_t type = aarch64_get_mem_u64 (ptr);
	u_int64_t state = aarch64_get_mem_u64 (ptr + 8);
	
	if (trace & TRACE_MISC)
	  fprintf (stderr, "Angel Exception: type 0x%" PRINT_64 "x status %" PRINT_64 "d\n",
		   type, state);

	return aarch64_set_status (type == 0x20026 ? STATUS_RETURN : STATUS_HALT, state);
      }
      break;
      
    case AngelSVC_Reason_Read:
    case AngelSVC_Reason_FLen:
    case AngelSVC_Reason_Seek:
    case AngelSVC_Reason_Remove:
    case AngelSVC_Reason_Time:
    case AngelSVC_Reason_System:
    case AngelSVC_Reason_Rename:
    case AngelSVC_Reason_Elapsed:
    default:
      if (trace & TRACE_MISC)
	fprintf (stderr, " HLT [Unknown angel %x]\n", aarch64_get_reg_u32 (0, NO_SP));
      return STATUS_HALT;
    }

  return aarch64_set_reg_u64 (0, NO_SP, result);
}

static StatusCode
dexExcpnGen (void)
{
  // assert instr[31:24] == 11010100
  // instr[23,21] = opc : 000 ==> GEN EXCPN, 001 ==> BRK
  //                      010 ==> HLT,       101 ==> DBG GEN EXCPN
  // instr[20,5] ==> imm16
  // instr[4,2] = opc2 000 ==> OK, ow ==> UNALLOC
  // instr[1,0] ==> LL : discriminates opc
  u_int32_t opc = uimm (instr, 23, 21);
  u_int32_t imm16 = uimm (instr, 20, 5);
  u_int32_t opc2 = uimm (instr, 4, 2);

  if (opc2 != 0)
    return_UNALLOC;

  u_int32_t LL = uimm (instr, 1, 0);

  // we only implement HLT and BRK for now
  if (opc == 1 && LL == 0)
    {
      if (trace & TRACE_MISC)
	fprintf (stderr, " BRK [0x%x]\n", imm16);
      return STATUS_BREAK;
    }

  if (opc == 2 && LL == 0)
    return handle_halt (imm16);

  else if (opc == 0 || opc == 5)
    return_NYI;

  return_UNALLOC;
}

static StatusCode
dexSystem (void)
{
  // assert instr[31:22] == 1101010100
  //
  // instr[21] = L
  // instr[20,19] = op0
  // instr[18,16] = op1
  // instr[15,12] = CRn
  // instr[11,8] = CRm
  // instr[7,5] = op2
  unsigned rt = uimm (instr, 4, 0);

  // we are interested in HINT, DSB, DMB and ISB
  //
  // Hint #0 encodes NOOP (this is the only hint we care about)
  // L == 0, op0 == 0, op1 = 011, CRn = 0010, Rt = 11111,
  // CRm op2  != 0000 000 OR CRm op2 == 0000 000 || CRm op > 0000 101
  //
  // DSB, DMB, ISB are data store barrier, data memory barrier and
  // instruction store barrier, respectively, where
  // 
  // L == 0, op0 == 0, op1 = 011, CRn = 0011, Rt = 11111,
  // op2 : DSB ==> 100, DMB ==> 101, ISB ==> 110
  // CRm<3:2> ==> domain, CRm<1:0> ==> types,
  // domain : 00 ==> OuterShareable, 01 ==> Nonshareable,
  //          10 ==> InerShareable, 11 ==> FullSystem
  // types :  01 ==> Reads, 10 ==> Writes,
  //          11 ==> All, 00 ==> All (domain == FullSystem)
  // 
  u_int32_t l_op0_op1_crn = uimm (instr, 21, 12);

  switch (l_op0_op1_crn)
    {
    case 0x032:
      if (rt == 0x1F)
	{
	  // NOP has CRm != 0000 OR
	  //         (CRm == 0000 AND (op2 == 000 OR op2 > 101))
	  u_int32_t crm = uimm (instr, 11, 8);
	  u_int32_t op2 = uimm (instr, 7, 5);

	  if (crm != 0 || (op2 == 0 || op2 > 5))
	    {
	      // actually call nop method so we can reimplement it later
	      nop ();
	      return status;
	    }
	}
      return_NYI;

    case 0x033:
      {
	u_int32_t op2 =  uimm (instr, 7, 5);

	switch (op2)
	  {
	  case 2: //clrex();
	    return_NYI;

	  case 4: return dsb ();
	  case 5: return dmb ();
	  case 6: return isb ();
	  case 7:
	  default: return_UNALLOC;
	}
      }

    case 0x3B0:
      // MRS Wt, sys-reg
      // FIXME: Ignore for now.
      return status;

    case 0x3B4:
    case 0x3BD:
      // MRS Xt, sys-reg
      // FIXME: Ignore for now.
      return status;
      
    case 0x0B7:
      // DC <type>, x<n>
      // FIXME: Ignore for now.
      return status;

    default:
      return_NYI;
    }
}

static StatusCode
dexBr (void)
{
  // u_int32_t group = dispatchGroup (instr);
  // assert  group == GROUP_BREXSYS_1010 || group == GROUP_BREXSYS_1011
  // bits [31,29] of a BrExSys are the secondary dispatch vector
  u_int32_t group2 = dispatchBrExSys (instr);

  switch (group2)
    {
    case BR_IMM_000:
      return dexBranchImmediate ();

    case BR_IMMCMP_001:
      // compare has bit 25 clear while test has it set
      if (!uimm (instr, 25, 25))
	return dexCompareBranchImmediate ();
      return dexTestBranchImmediate ();

    case BR_IMMCOND_010:
      // this is a conditional branch if bit 25 is clear otherwise
      // unallocated
      if (!uimm (instr, 25, 25))
	return dexCondBranchImmediate ();
      return_UNALLOC;

    case BR_UNALLOC_011:
      return_UNALLOC;

    case BR_IMM_100:
      return dexBranchImmediate ();

    case BR_IMMCMP_101:
      // compare has bit 25 clear while test has it set
      if (!uimm (instr, 25, 25))
	return dexCompareBranchImmediate ();

      return dexTestBranchImmediate ();

    case BR_REG_110:
      // unconditional branch reg has bit 25 set
      if (uimm (instr, 25, 25))
	return dexBranchRegister ();

      // this includes both Excpn Gen, System and unalloc operations
      // We need to decode the Excpn Gen operation BRK so we can plant
      // debugger entry points
      // Excpn Gen operations have instr[24] = 0
      // we need to decode at least one of the System operations NOP
      // which is an alias for HINT #0
      // System operations have instr[24,22] = 100
      if (uimm (instr, 24, 24) == 0)
	return dexExcpnGen ();

      if (uimm (instr, 24, 22) == 4)
	return dexSystem ();

      return_UNALLOC;

    case BR_UNALLOC_111:
      return_UNALLOC;
    }
  // should never reach here
  return report_and_die (UNIMPLEMENTED_INSTRUCTION);
}

// TODO specific decode and execute for group AdvSIMD

//// Some inline helpers and macros to help with
//// the different memory addressing modes. These are
//// useful for ing both integral and floating values.

// load-acquire/store-release (non-exclusive)
// TODO?

// prefetch
// IGNORE


//// Data Processing Immediate

// address generation
// TODO?

// conditional comparison
// leave these for later

// TODO store 8 bit scaled unsigned 12 bit etc
// TODO store 16 bit scaled unsigned 12 bit etc

// TODO FP convert to/from floating point
//
// we probably just need truncation towards zero to support
// casts to int or long?
//
// TODO implement the other conversions

// TODO FP round to integral/ nearest integral floating

// TODO FP arithmetic

static StatusCode
aarch64_decode_and_execute (u_int64_t pc)
{
  /* We need to check if gdb wants an in here.  */
  // checkBreak ();

  u_int64_t group = dispatchGroup (instr);

  switch (group)
    {
    case GROUP_PSEUDO_0000:   return dexPseudo ();
    case GROUP_LDST_0100:     return dexLdSt ();
    case GROUP_DPREG_0101:    return dexDPReg ();
    case GROUP_LDST_0110:     return dexLdSt ();
    case GROUP_ADVSIMD_0111:  return dexAdvSIMD0 ();
    case GROUP_DPIMM_1000:    return dexDPImm ();
    case GROUP_DPIMM_1001:    return dexDPImm ();
    case GROUP_BREXSYS_1010:  return dexBr ();
    case GROUP_BREXSYS_1011:  return dexBr ();
    case GROUP_LDST_1100:     return dexLdSt ();
    case GROUP_DPREG_1101:    return dexDPReg ();
    case GROUP_LDST_1110:     return dexLdSt ();
    case GROUP_ADVSIMD_1111:  return dexAdvSIMD1 ();

    case GROUP_UNALLOC_0001:
    case GROUP_UNALLOC_0010:
    case GROUP_UNALLOC_0011:
      return_UNALLOC;
    }

  /* Should never reach here.  */
  return report_and_die (UNIMPLEMENTED_INSTRUCTION);
}

StatusCode
aarch64_step (void)
{
  u_int64_t pc = aarch64_get_PC ();
  StatusCode state;

  if (pc == TOP_LEVEL_RETURN_PC)
    return STATUS_RETURN;

  aarch64_set_next_PC (pc + 4);
  instr = aarch64_get_mem_u32 (pc);

  if (trace & ~ (TRACE_FUNCTIONS | TRACE_MISC))
    fprintf (stderr, " pc = %" PRINT_64 "x ", pc);
  else if (disas)
    fprintf (stderr, " %" PRINT_64 "x ", pc);

  if (disas)
    aarch64_print_insn (pc);

  state = aarch64_decode_and_execute (pc);

  if ((trace & ~ (TRACE_FUNCTIONS | TRACE_MISC)) || disas)
    fprintf (stderr, "\n");

  return state;
}

StatusCode
aarch64_run (void)
{
  // only start if we are actually ready to run
  while (status == STATUS_READY)
    {
      status = aarch64_step ();

      switch (status)
	{
	case STATUS_READY:
	  aarch64_update_PC ();
	  break;

	case STATUS_HALT:
	  // doBreak ();
	  break;

	case STATUS_BREAK:
	  aarch64_update_PC ();
	  status = STATUS_READY;
	  break;

	case STATUS_RETURN:
	case STATUS_ERROR:
	default:
	  // finished running
	  break;

	case STATUS_CALLOUT:
	  // hit a callout to host -- handle it automatically and then
	  // continue running if it returns in a READY state nextPC is
	  // already set to the following instruction.  The exception
	  // stub prolog will reset nextPC if it wants to redirect the
	  // return
	  //status = doCallout ();
	  break;
	}
    }
  
  return status;
}

int
aarch64_init (void)
{
  char * buf;
  int saved_trace = trace;
  u_int64_t pc = aarch64_get_start_pc ();
  u_int64_t heap = aarch64_get_heap_start ();
  u_int64_t stack = aarch64_get_stack_start ();

  if (trace & TRACE_MISC)
    fprintf (stderr, " Allocate combined stack/heap from %" BFD_VMA_FMT "x to %" BFD_VMA_FMT "x\n",
	     heap, stack);
  /* Create a stack/heap.  */
  buf = malloc (stack - heap);
  if (buf == NULL)
    {
      fprintf (stderr, "sim error: Failed to allocate %" PRINT_64 "x byte RAM area\n",
	       stack - heap);
      return 0;
    }
  mem_add_blk (heap, buf, stack - heap, TRUE);

  /* Install SP, FP and PC and set LR to -1 so we can detect a
     top-level return.  */
  trace = 0;
  aarch64_set_reg_u64 (SP, SP_OK, stack);
  aarch64_set_reg_u64 (FP, SP_OK, stack);
  aarch64_set_reg_u64 (LR, SP_OK, TOP_LEVEL_RETURN_PC);
  aarch64_set_next_PC (pc);
  aarch64_update_PC ();
  status = STATUS_READY;
  errorCode = ERROR_NONE;
  init_LIT_table ();

  trace = saved_trace;

  return 1;
}

const char *
aarch64_get_error_text (void)
{
  switch (errorCode)
    {
    case ERROR_NONE:           return "(no error)";
    case ERROR_UNALLOC:        return "unallocated instruction";
    case ERROR_STACK_OVERFLOW: return "stack overflow";
    case ERROR_CLIENT:         return "client";
    case ERROR_NYI:            return "Not Yet Implemented";
    case ERROR_EXCEPTION:      return "exception";
    default:                   return "(unknown)";
    }
}
