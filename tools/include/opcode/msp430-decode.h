/* Opcode decoder for the TI MSP430
   Copyright 2012
   Free Software Foundation, Inc.
   Written by DJ Delorie <dj@redhat.com>

   This file is part of GDB, the GNU Debugger.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

typedef enum {
  MSO_unknown,
  /* double-operand instructions - all repeat .REPEATS times. */
  MSO_mov,	/* dest = src */
  MSO_add,	/* dest += src */
  MSO_addc,	/* dest += src + carry */
  MSO_subc,	/* dest -= (src-1) + carry */
  MSO_sub,	/* dest -= src */
  MSO_cmp,	/* dest - src -> status */
  MSO_dadd,	/* dest += src (as BCD) */
  MSO_bit,	/* dest & src -> status */
  MSO_bic,	/* dest &= ~src (bit clear) */
  MSO_bis,	/* dest |= src (bit set, OR) */
  MSO_xor,	/* dest ^= src */
  MSO_and,	/* dest &= src */

  /* single-operand instructions */
  MSO_rrc,	/* rotate through carry, dest >>= .REPEATS */
  MSO_swpb,	/* swap lower bytes of operand */
  MSO_rra,	/* signed shift dest >>= .REPEATS */
  MSO_sxt,	/* sign extend lower byte */
  MSO_push,	/* push .REPEATS registers (or other op) starting at SRC going towards R0 */
  MSO_pop,	/* pop .REPEATS registers starting at DEST going towards R15 */
  MSO_call,
  MSO_reti,

  /* jumps */
  MSO_jmp,	/* pc = SRC if .COND true.  */

  /* Extended single-operand instructions */
  MSO_rru,	/* unsigned shift right, dest >>= .REPEATS */

} MSP430_Opcode_ID;

typedef enum {
  MSP430_Operand_None,
  MSP430_Operand_Immediate,
  MSP430_Operand_Register,
  MSP430_Operand_Indirect,
  MSP430_Operand_Indirect_Postinc
} MSP430_Operand_Type;

typedef enum {
  MSR_0 = 0,
  MSR_PC = 0,
  MSR_SP = 1,
  MSR_SR = 2,
  MSR_CG = 3,
  MSR_None = 16,
} MSP430_Register;

typedef struct {
  MSP430_Operand_Type  type;
  int              addend;
  MSP430_Register  reg : 8;
  MSP430_Register  reg2 : 8;
  unsigned char	   bit_number : 4;
  unsigned char	   condition : 3;
} MSP430_Opcode_Operand;

typedef enum
{
  MSP430_Byte = 0,
  MSP430_Word,
  MSP430_Addr
} MSP430_Size;

/* These numerically match the bit encoding */
typedef enum {
  MSC_nz = 0,
  MSC_z,
  MSC_nc,
  MSC_c,
  MSC_n,
  MSC_ge,
  MSC_l,
  MSC_true,
} MSP430_Condition;

#define MSP430_FLAG_C	0x01
#define MSP430_FLAG_Z	0x02
#define MSP430_FLAG_N	0x04
#define MSP430_FLAG_V	0x80

typedef struct
{
  int lineno;
  MSP430_Opcode_ID	id;
  unsigned		flags_1:8;	/* These flags are set to '1' by the insn.  */
  unsigned		flags_0:8;	/* These flags are set to '0' by the insn.  */
  unsigned		flags_set:8;	/* These flags are set appropriately by the insn.  */
  unsigned		zc:1;		/* if set, pretend the carry bit is zero */
  unsigned		repeat_reg:1;	/* if set, count is in REG[repeats] */
  unsigned		repeats:6;	/* contains COUNT-1, or register number.  */
  int			n_bytes;	/* opcode size in BYTES */
  char *		syntax;
  MSP430_Size		size;		/* operand size in BITS */
  MSP430_Condition	cond;
  /* By convention, these are [0]destination, [1]source.  */
  MSP430_Opcode_Operand	op[2];
} MSP430_Opcode_Decoded;

int msp430_decode_opcode (unsigned long, MSP430_Opcode_Decoded *, int (*)(void *), void *);
