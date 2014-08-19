/* cpustate.h -- Prototypes for AArch64 simulator functions.

   Copyright (C) 2013 by Red Hat, Inc.

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

#include <stdio.h>

#include "cpustate.h"
#include "simulator.h"

static u_int64_t pc;
static u_int64_t nextpc;
static u_int32_t CPSR;
static u_int32_t FPSR;

static GRegister     gr[33];	// extra register at index 32 is used to hold zero value
static FRegister     fr[32];

/* Some operands are allowed to access the stack pointer (reg 31).
   For others a read from r31 always returns 0, and a write to r31 is ignored.  */
#define reg_num(reg) ((reg == R31 && !r31_is_sp) ? 32 : reg)

StatusCode
aarch64_set_reg_u64 (GReg reg, int r31_is_sp, u_int64_t val)
{
  if (reg == R31 && ! r31_is_sp)
    return aarch64_get_status ();

  if ((trace & TRACE_REG_WRITES) && val != gr[reg].u64)
    fprintf (stderr, "  GR[%2d] changes from %16" PRINT_64 "x to %16" PRINT_64 "x",
	     reg, gr[reg].u64, val);
  
  gr[reg].u64 = val;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_reg_s64 (GReg reg, int r31_is_sp, int64_t val)
{
  if (reg == R31 && ! r31_is_sp)
    return aarch64_get_status ();
  
  if ((trace & TRACE_REG_WRITES) && val != gr[reg].s64)
    fprintf (stderr, "  GR[%2d] changes from %16" PRINT_64 "x to %16" PRINT_64 "x",
	     reg, gr[reg].s64, val);
  
  gr[reg].s64 = val;
  return aarch64_get_status ();
}

u_int64_t
aarch64_get_reg_u64 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].u64;
}

int64_t
aarch64_get_reg_s64 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].s64;
}

u_int32_t
aarch64_get_reg_u32 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].u32;
}

int32_t
aarch64_get_reg_s32 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].s32;
}

u_int32_t
aarch64_get_reg_u16 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].u16;
}

int32_t
aarch64_get_reg_s16 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].s16;
}

u_int32_t
aarch64_get_reg_u8 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].u8;
}

int32_t
aarch64_get_reg_s8 (GReg reg, int r31_is_sp)
{
  return gr[reg_num(reg)].s8;
}

u_int64_t
aarch64_get_PC (void)
{
  return pc;
}

u_int64_t
aarch64_get_next_PC (void)
{
  return nextpc;
}

StatusCode
aarch64_set_next_PC (u_int64_t next)
{
  if ((trace & TRACE_REG_WRITES) && next != nextpc + 4)
    fprintf (stderr, "  NextPC changes from %16" PRINT_64 "x to %16" PRINT_64 "x", nextpc, next);
  
  nextpc = next;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_next_PC_by_offset (int64_t offset)
{
  if ((trace & TRACE_REG_WRITES) && pc + offset != nextpc + 4)
    fprintf (stderr, "  NextPC changes from %16" PRINT_64 "x to %16" PRINT_64 "x", nextpc, pc + offset);
  
  nextpc = pc + offset;
  return aarch64_get_status ();
}

// Install nextpc as current pc.
StatusCode
aarch64_update_PC (void)
{
  pc = nextpc;
  // Rezero the register we hand out when asked for ZR just in case it
  // was used as the destination for a write by the previous.
  // instruction
  gr[32].u64 = 0UL;
  return aarch64_get_status ();
}

// This instruction can be used to save the next PC to LR
// just before installing a branch PC.
StatusCode
aarch64_save_LR (void)
{
  if ((trace & TRACE_REG_WRITES) && gr[LR].u64 != nextpc)
    fprintf (stderr, "  LR    changes from %16" PRINT_64 "x to %16" PRINT_64 "x", gr[LR].u64, nextpc);
  
  gr[LR].u64 = nextpc;
  return aarch64_get_status ();
}

static const char *
decode_cpsr (FlagMask flags)
{
  switch (flags & CPSR_ALL_FLAGS)
    {
    default:
    case 0:  return "----";
    case 1:  return "---V";
    case 2:  return "--C-";
    case 3:  return "--CV";
    case 4:  return "-Z--";
    case 5:  return "-Z-V";
    case 6:  return "-ZC-";
    case 7:  return "-ZCV";
    case 8:  return "N---";
    case 9:  return "N--V";
    case 10: return "N-C-";
    case 11: return "N-CV";
    case 12: return "NZ--";
    case 13: return "NZ-V";
    case 14: return "NZC-";
    case 15: return "NZCV";
    }
}

// retrieve the CPSR register as an int
u_int32_t
aarch64_get_CPSR (void)
{
  return CPSR;
}

// set the CPSR register as an int
StatusCode
aarch64_set_CPSR (u_int32_t new_flags)
{
  if (trace & TRACE_REG_WRITES)
    {
      if (CPSR != new_flags)
	fprintf (stderr, "  CPSR changes from %s to %s",
		 decode_cpsr (CPSR), decode_cpsr (new_flags));
      else
	fprintf (stderr, "  CPSR stays at %s", decode_cpsr (CPSR));
    }	
  
  CPSR = new_flags & CPSR_ALL_FLAGS;
  return aarch64_get_status ();
}

// read a specific subset of the CPSR as a bit pattern
u_int32_t
aarch64_get_CPSR_bits (FlagMask mask)
{
  return CPSR & mask;
}

// assign a specific subset of the CPSR as a bit pattern
StatusCode
aarch64_set_CPSR_bits (u_int32_t mask, u_int32_t value)
{
  u_int32_t old_flags = CPSR;

  mask &= CPSR_ALL_FLAGS;
  CPSR &= ~ mask;
  CPSR |= (value & mask);

  if ((trace & TRACE_REG_WRITES) && old_flags != CPSR)
    fprintf (stderr, "  CPSR changes from %s to %s",
	     decode_cpsr (old_flags), decode_cpsr (CPSR));
  return aarch64_get_status ();
}

// test the value of a single CPSR returned as non-zero or zero
u_int32_t
aarch64_test_CPSR_bit (FlagMask bit)
{
  return CPSR & bit;
}

// set a single flag in the CPSR
StatusCode
aarch64_set_CPSR_bit (FlagMask bit)
{
  u_int32_t old_flags = CPSR;

  CPSR |= (bit & CPSR_ALL_FLAGS);

  if ((trace & TRACE_REG_WRITES) && old_flags != CPSR)
    fprintf (stderr, "  CPSR changes from %s to %s",
	     decode_cpsr (old_flags), decode_cpsr (CPSR));
  return aarch64_get_status ();
}

// clear a single flag in the CPSR
StatusCode
aarch64_clear_CPSR_bit (FlagMask bit)
{
  u_int32_t old_flags = CPSR;

  CPSR &= ~(bit & CPSR_ALL_FLAGS);

  if ((trace & TRACE_REG_WRITES) && old_flags != CPSR)
    fprintf (stderr, "  CPSR changes from %s to %s",
	     decode_cpsr (old_flags), decode_cpsr (CPSR));
  return aarch64_get_status ();
}

float
aarch64_get_FP_float (VReg reg)
{
  return fr[reg].s;
}

double
aarch64_get_FP_double (VReg reg)
{
  return fr[reg].d;
}

void
aarch64_get_FP_long_double (VReg reg, FRegister * a)
{
  a->v[0] = fr[reg].v[0];
  a->v[1] = fr[reg].v[1];
}

StatusCode
aarch64_set_FP_float (VReg reg, float val)
{
  if ((trace & TRACE_REG_WRITES) && val != fr[reg].s)
    fprintf (stderr, "  FR[%d] changes from %f to %f", reg, fr[reg].s, val);
  
  fr[reg].s = val;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_FP_double (VReg reg, double val)
{
  if ((trace & TRACE_REG_WRITES) && val != fr[reg].d)
    fprintf (stderr, "  FR[%d] changes from %f to %f", reg, fr[reg].d, val);

  fr[reg].d = val;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_FP_long_double (VReg reg, FRegister a)
{
  if ((trace & TRACE_REG_WRITES) /*&& val != fr[reg].q*/)
    {
      fprintf (stderr, "  FR[%d] changes from", reg);
      fprintf (stderr, " [%0lx", fr[reg].v[0]);
      fprintf (stderr, " %0lx]", fr[reg].v[1]);
      fprintf (stderr, "to [%lx", a.v[0]);
      fprintf (stderr, " %lx] ", a.v[1]);
    }
  
  fr[reg].v[0] = a.v[0];
  fr[reg].v[1] = a.v[1];
  return aarch64_get_status ();
}

u_int64_t
aarch64_get_vec_u64 (VReg reg, unsigned element)
{
  return fr[reg].v[element];
}

u_int32_t
aarch64_get_vec_u32 (VReg regno, unsigned element)
{
  return fr[regno].w[element];
}

u_int16_t
aarch64_get_vec_u16 (VReg regno, unsigned element)
{
  return fr[regno].h[element];
}

u_int8_t
aarch64_get_vec_u8 (VReg regno, unsigned element)
{
  return fr[regno].b[element];
}

StatusCode
aarch64_set_vec_u64 (VReg regno, unsigned element, u_int64_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].v[element])
    fprintf (stderr, "  VR[%2d].<long>[%d] changes from %16" PRINT_64 "x to %16" PRINT_64 "x",
	     regno, element, fr[regno].v[element], value);

  fr[regno].v[element] = value;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_vec_u32 (VReg regno, unsigned element, u_int32_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].w[element])
    fprintf (stderr, "  VR[%2d].<word>[%d] changes from %8x to %8x",
	     regno, element, fr[regno].w[element], value);

  fr[regno].w[element] = value;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_vec_u16 (VReg regno, unsigned element, u_int16_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].h[element])
    fprintf (stderr, "  VR[%2d].<half>[%d] changes from %4x to %4x",
	     regno, element, fr[regno].h[element], value);

  fr[regno].h[element] = value;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_vec_u8 (VReg regno, unsigned element, u_int8_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].b[element])
    fprintf (stderr, "  VR[%2d].<byte>[%d] changes from %x to %x",
	     regno, element, fr[regno].b[element], value);

  fr[regno].b[element] = value;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_FPSR (u_int32_t value)
{
  if ((trace & TRACE_REG_WRITES) && FPSR != value)
    fprintf (stderr, "  FPSR changes from %x to %x", FPSR, value);

  FPSR = value & FPSR_ALL_FPSRS;
  return aarch64_get_status ();
}

u_int32_t
aarch64_get_FPSR (void)
{
  return FPSR;
}

StatusCode
aarch64_set_FPSR_bits (u_int32_t mask, u_int32_t value)
{
  u_int32_t old_FPSR = FPSR;

  mask &= FPSR_ALL_FPSRS;
  FPSR &= ~mask;
  FPSR |= (value & mask);

  if ((trace & TRACE_REG_WRITES) && FPSR != old_FPSR)
    fprintf (stderr, "  FPSR changes from %x to %x", old_FPSR, FPSR);
  return aarch64_get_status ();
}

u_int32_t
aarch64_get_FPSR_bits (u_int32_t mask)
{
  mask &= FPSR_ALL_FPSRS;
  return FPSR & mask;
}

int
aarch64_test_FPSR_bit (FPSRMask flag)
{
  return FPSR & flag;
}

float
aarch64_get_vec_float (VReg v, unsigned e)
{
  return fr[v].S[e];
}

double
aarch64_get_vec_double (VReg v, unsigned e)
{
  return fr[v].D[e];
}

StatusCode
aarch64_set_vec_float (VReg v, unsigned e, float f)
{
  if ((trace & TRACE_REG_WRITES) && f != fr[v].S[e])
    fprintf (stderr, "  VR[%2d].<float>[%d] changes from %f to %f",
	     v, e, fr[v].S[e], f);
  fr[v].S[e] = f;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_vec_double (VReg v, unsigned e, double d)
{
  if ((trace & TRACE_REG_WRITES) && d != fr[v].D[e])
    fprintf (stderr, "  VR[%2d].<double>[%d] changes from %f to %f",
	     v, e, fr[v].D[e], d);
  fr[v].D[e] = d;
  return aarch64_get_status ();
}

int64_t
aarch64_get_vec_s64 (VReg regno, unsigned element)
{
  return fr[regno].V[element];
}

int32_t
aarch64_get_vec_s32 (VReg regno, unsigned element)
{
  return fr[regno].W[element];
}

int16_t
aarch64_get_vec_s16 (VReg regno, unsigned element)
{
  return fr[regno].H[element];
}

int8_t
aarch64_get_vec_s8 (VReg regno, unsigned element)
{
  return fr[regno].B[element];
}

StatusCode
aarch64_set_vec_s64 (VReg regno, unsigned element, int64_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].V[element])
    fprintf (stderr, "  VR[%2d].<long>[%d] changes from %16" PRINT_64 "x to %16" PRINT_64 "x",
	     regno, element, fr[regno].V[element], value);

  fr[regno].V[element] = value;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_vec_s32 (VReg regno, unsigned element, int32_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].W[element])
    fprintf (stderr, "  VR[%2d].<word>[%d] changes from %8x to %8x",
	     regno, element, fr[regno].W[element], value);

  fr[regno].W[element] = value;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_vec_s16 (VReg regno, unsigned element, int16_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].H[element])
    fprintf (stderr, "  VR[%2d].<half>[%d] changes from %4x to %4x",
	     regno, element, fr[regno].H[element], value);

  fr[regno].H[element] = value;
  return aarch64_get_status ();
}

StatusCode
aarch64_set_vec_s8 (VReg regno, unsigned element, int8_t value)
{
  if ((trace & TRACE_REG_WRITES) && value != fr[regno].B[element])
    fprintf (stderr, "  VR[%2d].<byte>[%d] changes from %x to %x",
	     regno, element, fr[regno].B[element], value);

  fr[regno].B[element] = value;
  return aarch64_get_status ();
}
