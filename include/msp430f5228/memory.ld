/* ============================================================================ */
/* Copyright (c) 2013, Texas Instruments Incorporated                           */
/*  All rights reserved.                                                        */
/*                                                                              */
/*  Redistribution and use in source and binary forms, with or without          */
/*  modification, are permitted provided that the following conditions          */
/*  are met:                                                                    */
/*                                                                              */
/*  *  Redistributions of source code must retain the above copyright           */
/*     notice, this list of conditions and the following disclaimer.            */
/*                                                                              */
/*  *  Redistributions in binary form must reproduce the above copyright        */
/*     notice, this list of conditions and the following disclaimer in the      */
/*     documentation and/or other materials provided with the distribution.     */
/*                                                                              */
/*  *  Neither the name of Texas Instruments Incorporated nor the names of      */
/*     its contributors may be used to endorse or promote products derived      */
/*     from this software without specific prior written permission.            */
/*                                                                              */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" */
/*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,       */
/*  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR      */
/*  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR            */
/*  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,       */
/*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,         */
/*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; */
/*  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,    */
/*  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR     */
/*  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,              */
/*  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                          */
/* ============================================================================ */

/* This file supports MSP430F5228 devices. */
/* Default linker script, for normal executables */

OUTPUT_ARCH(msp430)
ENTRY(_start)

/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
MEMORY {
  sfr              : ORIGIN = 0x0000, LENGTH = 0x0010 /* END=0x0010, size 16 */
  peripheral_8bit  : ORIGIN = 0x0010, LENGTH = 0x00F0 /* END=0x0100, size 240 */
  peripheral_16bit : ORIGIN = 0x0100, LENGTH = 0x0100 /* END=0x0200, size 256 */
  RAM (wx)         : ORIGIN = 0x2400, LENGTH = 0x2000 /* END=0x43FF, size 1K */
  infomem          : ORIGIN = 0x1800, LENGTH = 0x0200 /* END=0x19FF, size 512 as 4 128-byte segments */
  infod            : ORIGIN = 0x1800, LENGTH = 0x0080 /* END=0x187F, size 128 */
  infoc            : ORIGIN = 0x1880, LENGTH = 0x0080 /* END=0x18FF, size 128 */
  infob            : ORIGIN = 0x1900, LENGTH = 0x0080 /* END=0x197F, size 128 */
  infoa            : ORIGIN = 0x1980, LENGTH = 0x0080 /* END=0x19FF, size 128 */
  ROM (rx)         : ORIGIN = 0x4400, LENGTH = 0xBB80 /* END=0xFF7F, size 48000 */
  VECT1  (w)       : ORIGIN = 0xFF80, LENGTH = 0x0002
  VECT2  (w)       : ORIGIN = 0xFF82, LENGTH = 0x0002
  VECT3  (w)       : ORIGIN = 0xFF84, LENGTH = 0x0002
  VECT4  (w)       : ORIGIN = 0xFF86, LENGTH = 0x0002
  VECT5  (w)       : ORIGIN = 0xFF88, LENGTH = 0x0002
  VECT6  (w)       : ORIGIN = 0xFF8A, LENGTH = 0x0002
  VECT7  (w)       : ORIGIN = 0xFF8C, LENGTH = 0x0002
  VECT8  (w)       : ORIGIN = 0xFF8E, LENGTH = 0x0002
  VECT9  (w)       : ORIGIN = 0xFF90, LENGTH = 0x0002
  VECT10 (w)       : ORIGIN = 0xFF92, LENGTH = 0x0002
  VECT11 (w)       : ORIGIN = 0xFF94, LENGTH = 0x0002
  VECT12 (w)       : ORIGIN = 0xFF96, LENGTH = 0x0002
  VECT13 (w)       : ORIGIN = 0xFF98, LENGTH = 0x0002
  VECT14 (w)       : ORIGIN = 0xFF9A, LENGTH = 0x0002
  VECT15 (w)       : ORIGIN = 0xFF9C, LENGTH = 0x0002
  VECT16 (w)       : ORIGIN = 0xFF9E, LENGTH = 0x0002
  VECT17 (w)       : ORIGIN = 0xFFA0, LENGTH = 0x0002
  VECT18 (w)       : ORIGIN = 0xFFA2, LENGTH = 0x0002
  VECT19 (w)       : ORIGIN = 0xFFA4, LENGTH = 0x0002
  VECT20 (w)       : ORIGIN = 0xFFA6, LENGTH = 0x0002
  VECT21 (w)       : ORIGIN = 0xFFA8, LENGTH = 0x0002
  VECT22 (w)       : ORIGIN = 0xFFAA, LENGTH = 0x0002
  VECT23 (w)       : ORIGIN = 0xFFAC, LENGTH = 0x0002
  VECT24 (w)       : ORIGIN = 0xFFAE, LENGTH = 0x0002
  VECT25 (w)       : ORIGIN = 0xFFB0, LENGTH = 0x0002
  VECT26 (w)       : ORIGIN = 0xFFB2, LENGTH = 0x0002
  VECT27 (w)       : ORIGIN = 0xFFB4, LENGTH = 0x0002
  VECT28 (w)       : ORIGIN = 0xFFB6, LENGTH = 0x0002
  VECT29 (w)       : ORIGIN = 0xFFB8, LENGTH = 0x0002
  VECT30 (w)       : ORIGIN = 0xFFBA, LENGTH = 0x0002
  VECT31 (w)       : ORIGIN = 0xFFBC, LENGTH = 0x0002
  VECT32 (w)       : ORIGIN = 0xFFBE, LENGTH = 0x0002
  VECT33 (w)       : ORIGIN = 0xFFC0, LENGTH = 0x0002
  VECT34 (w)       : ORIGIN = 0xFFC2, LENGTH = 0x0002
  VECT35 (w)       : ORIGIN = 0xFFC4, LENGTH = 0x0002
  VECT36 (w)       : ORIGIN = 0xFFC6, LENGTH = 0x0002
  VECT37 (w)       : ORIGIN = 0xFFC8, LENGTH = 0x0002
  VECT38 (w)       : ORIGIN = 0xFFCA, LENGTH = 0x0002
  VECT39 (w)       : ORIGIN = 0xFFCC, LENGTH = 0x0002
  VECT40 (w)       : ORIGIN = 0xFFCE, LENGTH = 0x0002
  VECT41 (w)       : ORIGIN = 0xFFD0, LENGTH = 0x0002
  VECT42 (w)       : ORIGIN = 0xFFD2, LENGTH = 0x0002
  VECT43 (w)       : ORIGIN = 0xFFD4, LENGTH = 0x0002
  VECT44 (w)       : ORIGIN = 0xFFD6, LENGTH = 0x0002
  VECT45 (w)       : ORIGIN = 0xFFD8, LENGTH = 0x0002
  VECT46 (w)       : ORIGIN = 0xFFDA, LENGTH = 0x0002
  VECT47 (w)       : ORIGIN = 0xFFDC, LENGTH = 0x0002
  VECT48 (w)       : ORIGIN = 0xFFDE, LENGTH = 0x0002
  VECT49 (w)       : ORIGIN = 0xFFE0, LENGTH = 0x0002
  VECT50 (w)       : ORIGIN = 0xFFE2, LENGTH = 0x0002
  VECT51 (w)       : ORIGIN = 0xFFE4, LENGTH = 0x0002
  VECT52 (w)       : ORIGIN = 0xFFE6, LENGTH = 0x0002
  VECT53 (w)       : ORIGIN = 0xFFE8, LENGTH = 0x0002
  VECT54 (w)       : ORIGIN = 0xFFEA, LENGTH = 0x0002
  VECT55 (w)       : ORIGIN = 0xFFEC, LENGTH = 0x0002
  VECT56 (w)       : ORIGIN = 0xFFEE, LENGTH = 0x0002
  VECT57 (w)       : ORIGIN = 0xFFF0, LENGTH = 0x0002
  VECT58 (w)       : ORIGIN = 0xFFF2, LENGTH = 0x0002
  VECT59 (w)       : ORIGIN = 0xFFF4, LENGTH = 0x0002
  VECT60 (w)       : ORIGIN = 0xFFF6, LENGTH = 0x0002
  VECT61 (w)       : ORIGIN = 0xFFF8, LENGTH = 0x0002
  VECT62 (w)       : ORIGIN = 0xFFFA, LENGTH = 0x0002
  VECT63 (w)       : ORIGIN = 0xFFFC, LENGTH = 0x0002
  RESETVEC (w)     : ORIGIN = 0xFFFE, LENGTH = 0x0002
  bsl              : ORIGIN = 0x1000, LENGTH = 0x0800
  ram2 (wx)        : ORIGIN = 0x0000, LENGTH = 0x0000
  ram_mirror (wx)  : ORIGIN = 0x0000, LENGTH = 0x0000
  usbram (wx)      : ORIGIN = 0x0000, LENGTH = 0x0000
  far_rom          : ORIGIN = 0x00010000, LENGTH = 0x0000C3FF
}

