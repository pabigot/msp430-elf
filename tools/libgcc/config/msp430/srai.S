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

	.macro	_srai n
	.global __mspabi_srai_\n
__mspabi_srai_\n:
	RRA.W	R12
	.endm

/* Logical Right Shift - R12 -> R12 */
	_srai	15
	_srai	14
	_srai	13
	_srai	12
	_srai	11
	_srai	10
	_srai	9
	_srai	8
	_srai	7
	_srai	6
	_srai	5
	_srai	4
	_srai	3
	_srai	2
	_srai	1
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

1:	ADD.W	#-1,R13
	RRA.W	R12,R12
	.global	__mspabi_srai
__mspabi_srai:
	CMP	#0,R13
	JNZ	1b
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

/* Logical Right Shift - R12:R13 -> R12:R13 */

	.macro	_sral	n
	.global	__mspabi_sral_\n
__mspabi_sral_\n:
	RRA.W	R13
	RRC.W	R12
	.endm

	_sral	15
	_sral	14
	_sral	13
	_sral	12
	_sral	11
	_sral	10
	_sral	9
	_sral	8
	_sral	7
	_sral	6
	_sral	5
	_sral	4
	_sral	3
	_sral	2
	_sral	1
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

1:	ADD.W	#-1,R14
	RRA.W	R13
	RRC.W	R12
	.global	__mspabi_sral
__mspabi_sral:
	CMP	#0,R14
	JNZ	1b
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif
