// rx.h -- ELF definitions specific to EM_RX  -*- C++ -*-

// Copyright 2010, Free Software Foundation, Inc.
// Written by Nick Clifton <nickc@redhat.com>

// This file is part of elfcpp.
   
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation; either version 3, or
// (at your option) any later version.

// In addition to the permissions in the GNU Library General Public
// License, the Free Software Foundation gives you unlimited
// permission to link the compiled version of this file into
// combinations with other programs, and to distribute those
// combinations without any restriction coming from the use of this
// file.  (The Library Public License restrictions do apply in other
// respects; for example, they cover modification of the file, and
/// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef ELFCPP_RX_H
#define ELFCPP_RX_H

namespace elfcpp
{

  // RX Relocations Codes
  enum
  {
    R_RX_NONE =        0x00,
    R_RX_DIR32 =       0x01,
    R_RX_DIR24S =      0x02,
    R_RX_DIR16 =       0x03,
    R_RX_DIR16U =      0x04,
    R_RX_DIR16S =      0x05,
    R_RX_DIR8 =        0x06,
    R_RX_DIR8U =       0x07,
    R_RX_DIR8S =       0x08,
    R_RX_DIR24S_PCREL = 0x9,
    R_RX_DIR16S_PCREL = 0xa,
    R_RX_DIR8S_PCREL = 0x0b,
    R_RX_DIR16UL =     0x0c,
    R_RX_DIR16UW =     0x0d,
    R_RX_DIR8UL =      0x0e,
    R_RX_DIR8UW =      0x0f,
    R_RX_DIR32_REV =   0x10,
    R_RX_DIR16_REV =   0x11,
    R_RX_DIR3U_PCREL = 0x12,

    // These are extensions added by Red Hat.
    R_RX_RH_3_PCREL =  0x20, // Like R_RX_DIR8S_PCREL but only 3-bits.
    R_RX_RH_16_OP =    0x21, // Like R_RX_DIR16 but for opcodes - always big endian.
    R_RX_RH_24_OP =    0x22, // Like R_RX_DIR24S but for opcodes - always big endian.
    R_RX_RH_32_OP =    0x23, // Like R_RX_DIR32 but for opcodes - always big endian.
    R_RX_RH_24_UNS =   0x24, // Like R_RX_DIR24S but for unsigned values.
    R_RX_RH_8_NEG =    0x25, // Like R_RX_DIR8 but -x is stored.
    R_RX_RH_16_NEG =   0x26, // Like R_RX_DIR16 but -x is stored.
    R_RX_RH_24_NEG =   0x27, // Like R_RX_DIR24S but -x is stored.
    R_RX_RH_32_NEG =   0x28, // Like R_RX_DIR32 but -x is stored.
    R_RX_RH_DIFF =     0x29, // Subtract from a previous relocation.
    R_RX_RH_GPRELB =   0x2a, // Byte value, relative to __gp.
    R_RX_RH_GPRELW =   0x2b, // Word value, relative to __gp.
    R_RX_RH_GPRELL =   0x2c, // Long value, relative to __gp.
    R_RX_RH_RELAX =    0x2d, // Marks opcodes suitable for linker relaxation.

    // These are for complex relocs.
    R_RX_ABS32 =       0x41,
    R_RX_ABS24S =      0x42,
    R_RX_ABS16 =       0x43,
    R_RX_ABS16U =      0x44,
    R_RX_ABS16S =      0x45,
    R_RX_ABS8 =        0x46,
    R_RX_ABS8U =       0x47,
    R_RX_ABS8S =       0x48,
    R_RX_ABS24S_PCREL = 0x49,
    R_RX_ABS16S_PCREL = 0x4a,
    R_RX_ABS8S_PCREL = 0x4b,
    R_RX_ABS16UL =     0x4c,
    R_RX_ABS16UW =     0x4d,
    R_RX_ABS8UL =      0x4e,
    R_RX_ABS8UW =      0x4f,
    R_RX_ABS32_REV =   0x50,
    R_RX_ABS16_REV =   0x51,
  
    R_RX_SYM =         0x80,
    R_RX_OPneg =       0x81,
    R_RX_OPadd =       0x82,
    R_RX_OPsub =       0x83,
    R_RX_OPmul =       0x84,
    R_RX_OPdiv =       0x85,
    R_RX_OPshla =      0x86,
    R_RX_OPshra =      0x87,
    R_RX_OPsctsize =   0x88,
    R_RX_OPscttop =    0x8d,
    R_RX_OPand =       0x90,
    R_RX_OPor =        0x91,
    R_RX_OPxor =       0x92,
    R_RX_OPnot =       0x93,
    R_RX_OPmod =       0x94,
    R_RX_OPromtop =    0x95,
    R_RX_OPramtop =    0x96,
  };

  // e_flags values defined for RX
  enum
  {
    E_FLAG_RX_64BIT_DOUBLES = (1 << 0),
    E_FLAG_RX_DSP	    = (1 << 1),
  };

  // These define the addend field of R_RX_RH_RELAX relocations.
  enum
  {
    RX_RELAXA_IMM6 =	0x00000010,	// Imm8/16/24/32 at bit offset 6.
    RX_RELAXA_IMM12 =   0x00000020,	// Imm8/16/24/32 at bit offset 12.
    RX_RELAXA_DSP4 =	0x00000040,	// Dsp0/8/16 at bit offset 4.
    RX_RELAXA_DSP6 =	0x00000080,	// Dsp0/8/16 at bit offset 6.
    RX_RELAXA_DSP14 =	0x00000100,	// Dsp0/8/16 at bit offset 14.
    RX_RELAXA_BRA =	0x00000200,	// Any type of branch (must be decoded).
    RX_RELAXA_RNUM =	0x0000000f,	// Number of associated relocations.
    RX_RELAXA_ALIGN =   0x10000000,	// Start alignment; the remaining bits are the alignment value.
    RX_RELAXA_ELIGN =   0x20000000,	// End alignment; the remaining bits are the alignment value.
    RX_RELAXA_ANUM =	0x00ffffff,	// Alignment amount, in bytes (i.e. .balign).
  };
  
} // End namespace elfcpp.

#endif // !defined(ELFCPP_RX_H)
