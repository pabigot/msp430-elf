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

/* This file supports MSP430L092 devices. */
/* Default linker script, for normal executables */

OUTPUT_ARCH(msp430)
ENTRY(_start)

/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
MEMORY {
  sfr              : ORIGIN = 0x0000, LENGTH = 0x0010 /* END=0x0010, size 16 */
  peripheral_8bit  : ORIGIN = 0x0010, LENGTH = 0x00F0 /* END=0x0100, size 240 */
  peripheral_16bit : ORIGIN = 0x0100, LENGTH = 0x0100 /* END=0x0200, size 256 */
  RAM (wx)         : ORIGIN = 0x2380, LENGTH = 0x0080 /* END=0x23FF, size 1K */
  infomem          : ORIGIN = 0x1C00, LENGTH = 0x0060 /* END=0x1C5F, size 96 as 1 96-byte segments */
  infod            : ORIGIN = 0x1800, LENGTH = 0x0060 /* END=0x187F, size 96 */
  infoc            : ORIGIN = 0x1880, LENGTH = 0x0060 /* END=0x18FF, size 96 */
  infob            : ORIGIN = 0x1BA0, LENGTH = 0x0060 /* END=0x1BFF, size 96 */
  infoa            : ORIGIN = 0x1C00, LENGTH = 0x0060 /* END=0x1C5F, size 96 */
  ROM (rx)         : ORIGIN = 0x1C80, LENGTH = 0x0700 /* END=0x237F, size 1792 */
  VECT1  (w)       : ORIGIN = 0x1C60, LENGTH = 0x0002
  VECT2  (w)       : ORIGIN = 0x1C62, LENGTH = 0x0002
  VECT3  (w)       : ORIGIN = 0x1C64, LENGTH = 0x0002
  VECT4  (w)       : ORIGIN = 0x1C66, LENGTH = 0x0002
  VECT5  (w)       : ORIGIN = 0x1C68, LENGTH = 0x0002
  VECT6  (w)       : ORIGIN = 0x1C6A, LENGTH = 0x0002
  VECT7  (w)       : ORIGIN = 0x1C6C, LENGTH = 0x0002
  VECT8  (w)       : ORIGIN = 0x1C6E, LENGTH = 0x0002
  VECT9  (w)       : ORIGIN = 0x1C70, LENGTH = 0x0002
  VECT10 (w)       : ORIGIN = 0x1C72, LENGTH = 0x0002
  VECT11 (w)       : ORIGIN = 0x1C74, LENGTH = 0x0002
  VECT12 (w)       : ORIGIN = 0x1C76, LENGTH = 0x0002
  VECT13 (w)       : ORIGIN = 0x1C78, LENGTH = 0x0002
  VECT14 (w)       : ORIGIN = 0x1C7A, LENGTH = 0x0002
  VECT15 (w)       : ORIGIN = 0x1C7C, LENGTH = 0x0002
  RESETVEC (w)     : ORIGIN = 0x1C7E, LENGTH = 0x0002
  bsl              : ORIGIN = 0x0000, LENGTH = 0x0000
  ram2 (wx)        : ORIGIN = 0x0000, LENGTH = 0x0000
  ram_mirror (wx)  : ORIGIN = 0x0000, LENGTH = 0x0000
  usbram (wx)      : ORIGIN = 0x0000, LENGTH = 0x0000
  far_rom          : ORIGIN = 0x00000000, LENGTH = 0x00000000
}

