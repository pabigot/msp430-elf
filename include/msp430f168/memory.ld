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

/* This file supports MSP430F168 devices. */
/* Default linker script, for normal executables */

OUTPUT_ARCH(msp430)
ENTRY(_start)

/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
MEMORY {
  sfr              : ORIGIN = 0x0000, LENGTH = 0x0010 /* END=0x0010, size 16 */
  peripheral_8bit  : ORIGIN = 0x0010, LENGTH = 0x00F0 /* END=0x0100, size 240 */
  peripheral_16bit : ORIGIN = 0x0100, LENGTH = 0x0100 /* END=0x0200, size 256 */
  RAM (wx)         : ORIGIN = 0x0200, LENGTH = 0x0800 /* END=0x09FF, size 1K */
  infomem          : ORIGIN = 0x1000, LENGTH = 0x0100 /* END=0x10FF, size 256 as 2 128-byte segments */
  infod            : ORIGIN = 0x0001, LENGTH = 0x0080 /* END=0x0000, size 128 */
  infoc            : ORIGIN = 0x0F80, LENGTH = 0x0080 /* END=0x0FFF, size 128 */
  infob            : ORIGIN = 0x1000, LENGTH = 0x0080 /* END=0x107F, size 128 */
  infoa            : ORIGIN = 0x1080, LENGTH = 0x0080 /* END=0x10FF, size 128 */
  ROM (rx)         : ORIGIN = 0x4000, LENGTH = 0xBFE0 /* END=0xFFDF, size 49120 */
  VECT1  (w)       : ORIGIN = 0xFFE0, LENGTH = 0x0002
  VECT2  (w)       : ORIGIN = 0xFFE2, LENGTH = 0x0002
  VECT3  (w)       : ORIGIN = 0xFFE4, LENGTH = 0x0002
  VECT4  (w)       : ORIGIN = 0xFFE6, LENGTH = 0x0002
  VECT5  (w)       : ORIGIN = 0xFFE8, LENGTH = 0x0002
  VECT6  (w)       : ORIGIN = 0xFFEA, LENGTH = 0x0002
  VECT7  (w)       : ORIGIN = 0xFFEC, LENGTH = 0x0002
  VECT8  (w)       : ORIGIN = 0xFFEE, LENGTH = 0x0002
  VECT9  (w)       : ORIGIN = 0xFFF0, LENGTH = 0x0002
  VECT10 (w)       : ORIGIN = 0xFFF2, LENGTH = 0x0002
  VECT11 (w)       : ORIGIN = 0xFFF4, LENGTH = 0x0002
  VECT12 (w)       : ORIGIN = 0xFFF6, LENGTH = 0x0002
  VECT13 (w)       : ORIGIN = 0xFFF8, LENGTH = 0x0002
  VECT14 (w)       : ORIGIN = 0xFFFA, LENGTH = 0x0002
  VECT15 (w)       : ORIGIN = 0xFFFC, LENGTH = 0x0002
  RESETVEC (w)     : ORIGIN = 0xFFFE, LENGTH = 0x0002
  bsl              : ORIGIN = 0x0000, LENGTH = 0x0000
  ram2 (wx)        : ORIGIN = 0x0000, LENGTH = 0x0000
  ram_mirror (wx)  : ORIGIN = 0x0000, LENGTH = 0x0000
  usbram (wx)      : ORIGIN = 0x0000, LENGTH = 0x0000
  far_rom          : ORIGIN = 0x00000000, LENGTH = 0x00000000
}

