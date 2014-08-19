/* cpustate.h -- Prototypes for AArch64 cpu state functions.

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

#ifndef _CPU_STATE_H
#define _CPU_STATE_H

#include <sys/types.h>

#if __WORDSIZE == 64
#define PRINT_64 "l"
#else
#define PRINT_64 "ll"
#endif

typedef enum
{
  STATUS_READY   = 0, // may continue stepping or running
  STATUS_RETURN  = 1, // via normal return from initial frame
  STATUS_HALT    = 2, // via HALT pseudo-instruction
  STATUS_BREAK   = 3, // via BRK instruction
  STATUS_CALLOUT = 4, // via CALLOUT pseudo-instruction
  STATUS_ERROR   = 5, // simulator detected problem
  STATUS_MAX     = 6
} StatusCode;

/*
 * symbolic names used to identify general registers which also match
 * the registers indices in machine code
 *
 * We have 32 general registers which can be read/written as 32 bit or
 * 64 bit sources/sinks and are appropriately referred to as Wn or Xn
 * in the assembly code.  Some instructions mix these access modes
 * (e.g. ADD X0, X1, W2) so the implementation of the instruction
 * needs to *know* which type of read or write access is required.
 */
typedef enum GReg
{
  R0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  R16,
  R17,
  R18,
  R19,
  R20,
  R21,
  R22,
  R23,
  R24,
  R25,
  R26,
  R27,
  R28,
  R29,
  R30,
  R31,
  FP = R29,
  LR = R30,
  SP = R31,
  ZR = R31
} GReg;

/*
 * symbolic names used to refer to floating point registers which also
 * match the registers indices in machine code
 *
 * We have 32 FP registers which can be read/written as 8, 16, 32, 64
 * and 128 bit sources/sinks and are appropriately referred to as Bn,
 * Hn, Sn, Dn and Qn in the assembly code. Some instructions mix these
 * access modes (e.g. FCVT S0, D0) so the implementation of the
 * instruction needs to *know* which type of read or write access is
 * required.
 */

typedef enum VReg
{
  V0,
  V1,
  V2,
  V3,
  V4,
  V5,
  V6,
  V7,
  V8,
  V9,
  V10,
  V11,
  V12,
  V13,
  V14,
  V15,
  V16,
  V17,
  V18,
  V19,
  V20,
  V21,
  V22,
  V23,
  V24,
  V25,
  V26,
  V27,
  V28,
  V29,
  V30,
  V31,
} VReg;

/*
 * all the different integer bit patterns for the components of a
 * general register are overlaid here using a union so as to allow all
 * reading and writing of the desired bits.
 *
 * n.b. the ARM spec says that when you write a 32 bit register you
 * are supposed to write the low 32 bits and zero the high 32
 * bits. But we don't actually have to care about this because Java
 * will only ever consume the 32 bits value as a 64 bit quantity after
 * an explicit extend.
 */
typedef union GRegisterValue
{
  int8_t s8;
  int16_t s16;
  int32_t s32;
  int64_t s64;
  u_int8_t u8;
  u_int16_t u16;
  u_int32_t u32;
  u_int64_t u64;
} GRegister;

/*
 * float registers provide for storage of a single, double or quad
 * word format float in the same register. single floats are not
 * paired within each double register as per 32 bit arm. instead each
 * 128 bit register Vn embeds the bits for Sn, and Dn in the lower
 * quarter and half, respectively, of the bits for Qn.
 *
 * The upper bits can also be accessed as single or double floats by
 * the float vector operations using indexing e.g. V1.D[1], V1.S[3]
 * etc and, for SIMD operations using a horrible index range notation.
 *
 * The spec also talks about accessing float registers as half words
 * and bytes with Hn and Bn providing access to the low 16 and 8 bits
 * of Vn but it is not really clear what these bits represent. We can
 * probably ignore this for Java anyway. However, we do need to access
 * the raw bits at 32 and 64 bit resolution to load to/from integer
 * registers.
 *
 * Note - we do not use the long double type.  Aliasing issues between
 * integer and float values mean that it is unreliable to use them.
 */

typedef union FRegisterValue
{
  float        s;
  double       d;

  u_int64_t    v[2];
  u_int32_t    w[4];
  u_int16_t    h[8];
  u_int8_t     b[16];

  int64_t      V[2];
  int32_t      W[4];
  int16_t      H[8];
  int8_t       B[16];

  float        S[4];
  double       D[2];

} FRegister;

/*
 * condition register bit select values
 *
 * the order of bits here is important because some of
 * the flag setting conditional instructions employ a
 * bit field to populate the flags when a false condition
 * bypasses execution of the operation and we want to
 * be able to assign the flags register using the
 * supplied value.
 */

typedef enum FlagIdx
{
  V_IDX,
  C_IDX,
  Z_IDX,
  N_IDX
} FlagIdx;

typedef enum FlagMask
{
  V = 1 << V_IDX,
  C = 1 << C_IDX,
  Z = 1 << Z_IDX,
  N = 1 << N_IDX
} FlagMask;

#define CPSR_ALL_FLAGS (V | C | Z | N)

typedef u_int32_t FlagsRegister;

/*
 * FPSR register -- floating point status register
 
 * this register includes IDC, IXC, UFC, OFC, DZC, IOC and QC bits,
 * and the floating point N, Z, C, V bits but the latter are unused in
 * aarch64 mode. the sim ignores QC for now.
 *
 * bit positions are as per the ARMv7 FPSCR register
 *
 * IDC :  7 ==> Input Denormal (cumulative exception bit)
 * IXC :  4 ==> Inexact 
 * UFC :  3 ==> Underflow
 * OFC :  2 ==> Overflow
 * DZC :  1 ==> Division by Zero
 * IOC :  0 ==> Invalid Operation
 *
   The rounding mode is held in bits [23,22] defined as follows:

   0b00 Round to Nearest (RN) mode
   0b01 Round towards Plus Infinity (RP) mode
   0b10 Round towards Minus Infinity (RM) mode
   0b11 Round towards Zero (RZ) mode.
 */

// indices for bits in the FPSR register value
typedef enum FPSRIdx
{
  IO_IDX = 0,
  DZ_IDX = 1,
  OF_IDX = 2,
  UF_IDX = 3,
  IX_IDX = 4,
  ID_IDX = 7
} FPSRIdx;

// corresponding bits as numeric values
typedef enum FPSRMask
{
  IO = (1 << IO_IDX),
  DZ = (1 << DZ_IDX),
  OF = (1 << OF_IDX),
  UF = (1 << UF_IDX),
  IX = (1 << IX_IDX),
  ID = (1 << ID_IDX)
} FPSRMask;

#define FPSR_ALL_FPSRS (IO | DZ | OF | UF | IX | ID)


// General Register access functions.
extern u_int64_t   aarch64_get_reg_u64 (GReg, int);
extern int64_t     aarch64_get_reg_s64 (GReg, int);
extern u_int32_t   aarch64_get_reg_u32 (GReg, int);
extern int32_t     aarch64_get_reg_s32 (GReg, int);
extern u_int32_t   aarch64_get_reg_u16 (GReg, int);
extern int32_t     aarch64_get_reg_s16 (GReg, int);
extern u_int32_t   aarch64_get_reg_u8  (GReg, int);
extern int32_t     aarch64_get_reg_s8  (GReg, int);

extern StatusCode  aarch64_set_reg_u64 (GReg, int, u_int64_t);
extern StatusCode  aarch64_set_reg_s64 (GReg, int, int64_t);

// FP Register access functions.
extern float       aarch64_get_FP_float (VReg);
extern double      aarch64_get_FP_double (VReg);
extern void        aarch64_get_FP_long_double (VReg, FRegister *);
extern StatusCode  aarch64_set_FP_float (VReg, float);
extern StatusCode  aarch64_set_FP_double (VReg, double);
extern StatusCode  aarch64_set_FP_long_double (VReg, FRegister);

// PC register accessors.
extern u_int64_t   aarch64_get_PC (void);
extern u_int64_t   aarch64_get_next_PC (void);
extern StatusCode  aarch64_set_next_PC (u_int64_t);
extern StatusCode  aarch64_set_next_PC_by_offset (int64_t);
extern StatusCode  aarch64_update_PC (void);
extern StatusCode  aarch64_save_LR (void);

// Flag register accessors.
extern u_int32_t   aarch64_get_CPSR (void);
extern StatusCode  aarch64_set_CPSR (u_int32_t);
extern u_int32_t   aarch64_get_CPSR_bits (u_int32_t);
extern StatusCode  aarch64_set_CPSR_bits (u_int32_t, u_int32_t);
extern u_int32_t   aarch64_test_CPSR_bit (FlagMask);
extern StatusCode  aarch64_set_CPSR_bit (FlagMask);
extern StatusCode  aarch64_clear_CPSR_bit (FlagMask);

extern StatusCode  aarch64_set_FPSR (u_int32_t);
extern u_int32_t   aarch64_get_FPSR (void);
extern StatusCode  aarch64_set_FPSR_bits (u_int32_t, u_int32_t);
extern u_int32_t   aarch64_get_FPSR_bits (u_int32_t);
extern int         aarch64_test_FPSR_bit (FPSRMask);

// Vector register accessors.
extern u_int64_t   aarch64_get_vec_u64 (VReg, unsigned);
extern u_int32_t   aarch64_get_vec_u32 (VReg, unsigned);
extern u_int16_t   aarch64_get_vec_u16 (VReg, unsigned);
extern u_int8_t    aarch64_get_vec_u8  (VReg, unsigned);
extern StatusCode  aarch64_set_vec_u64 (VReg, unsigned, u_int64_t);
extern StatusCode  aarch64_set_vec_u32 (VReg, unsigned, u_int32_t);
extern StatusCode  aarch64_set_vec_u16 (VReg, unsigned, u_int16_t);
extern StatusCode  aarch64_set_vec_u8  (VReg, unsigned, u_int8_t);

extern int64_t     aarch64_get_vec_s64 (VReg, unsigned);
extern int32_t     aarch64_get_vec_s32 (VReg, unsigned);
extern int16_t     aarch64_get_vec_s16 (VReg, unsigned);
extern int8_t      aarch64_get_vec_s8  (VReg, unsigned);
extern StatusCode  aarch64_set_vec_s64 (VReg, unsigned, int64_t);
extern StatusCode  aarch64_set_vec_s32 (VReg, unsigned, int32_t);
extern StatusCode  aarch64_set_vec_s16 (VReg, unsigned, int16_t);
extern StatusCode  aarch64_set_vec_s8  (VReg, unsigned, int8_t);

extern float       aarch64_get_vec_float (VReg, unsigned);
extern double      aarch64_get_vec_double (VReg, unsigned);
extern StatusCode  aarch64_set_vec_float (VReg, unsigned, float);
extern StatusCode  aarch64_set_vec_double (VReg, unsigned, double);

#endif // ifndef _CPU_STATE_H
