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
	
	.text

/* Logical Left Shift - R12 -> R12 */

	.macro	_slli n
	.global __mspabi_slli_\n
__mspabi_slli_\n:
	ADD.W	R12,R12
	.endm

	_slli	15
	_slli	14
	_slli	13
	_slli	12
	_slli	11
	_slli	10
	_slli	9
	_slli	8
	_slli	7
	_slli	6
	_slli	5
	_slli	4
	_slli	3
	_slli	2
	_slli	1
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

1:	ADD.W	#-1,R13
	ADD.W	R12,R12
	.global	__mspabi_slli
__mspabi_slli:
	CMP	#0,R13
	JNZ	1b
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

/* Logical Left Shift - R12:R13 -> R12:R13 */

	.macro	_slll	n
	.global	__mspabi_slll_\n
__mspabi_slll_\n:
	ADD.W	R12,R12
	ADDC.W	R13,R13
	.endm

	_slll	15
	_slll	14
	_slll	13
	_slll	12
	_slll	11
	_slll	10
	_slll	9
	_slll	8
	_slll	7
	_slll	6
	_slll	5
	_slll	4
	_slll	3
	_slll	2
	_slll	1
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

1:	ADD.W	#-1,R14
	ADD.W	R12,R12
	ADDC.W	R13,R13
	.global	__mspabi_slll
__mspabi_slll:
	CMP	#0,R14
	JNZ	1b
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

