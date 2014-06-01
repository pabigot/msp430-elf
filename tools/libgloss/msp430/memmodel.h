/* Copyright (c) 2012 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met: 

     Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

     Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     The name of Red Hat Incorporated may not be used to endorse 
     or promote products derived from this software without specific 
     prior written permission.

   This software is provided by the copyright holders and contributors
   "AS IS" and any express or implied warranties, including, but not
   limited to, the implied warranties of merchantability and fitness for
   a particular purpose are disclaimed.  In no event shall Red Hat
   incorporated be liable for any direct, indirect, incidental, special,
   exemplary, or consequential damages (including, but not limited to,
   procurement of substitute goods or services; loss of use, data, or
   profits; or business interruption) however caused and on any theory of
   liability, whether in contract, strict liability, or tort (including
   negligence or otherwise) arising in any way out of the use of this
   software, even if advised of the possibility of such damage.  */

/* This file provides macros for various MSP430 instructions
   which have similar, but not identical, versions when assembling
   for the LARGE memory model.  */

#ifdef __MSP430X_LARGE__

#define call_	CALLA
#define ret_	RETA
#define mov_	MOVA
#define movx_	MOVX
#define br_	BRA
#define cmp_	CMPA
#define add_	ADDA
#define PTRsz	4

#else

#define call_	CALL
#define ret_	RET
#define mov_	MOV
#define movx_	MOV
#define br_	BR
#define cmp_	CMP
#define add_	ADD
#define PTRsz	2


#endif
