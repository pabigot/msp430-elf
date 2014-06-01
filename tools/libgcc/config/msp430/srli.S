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

	.macro	_srli n
	.global __mspabi_srli_\n
__mspabi_srli_\n:
	CLRC
	RRC.W	R12
	.endm

/* Logical Right Shift - R12 -> R12 */
	_srli	15
	_srli	14
	_srli	13
	_srli	12
	_srli	11
	_srli	10
	_srli	9
	_srli	8
	_srli	7
	_srli	6
	_srli	5
	_srli	4
	_srli	3
	_srli	2
	_srli	1
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

1:	ADD.W	#-1,R13
	CLRC
	RRC.W	R12,R12
	.global	__mspabi_srli
__mspabi_srli:
	CMP	#0,R13
	JNZ	1b
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

/* Logical Right Shift - R12:R13 -> R12:R13 */

	.macro	_srll	n
	.global	__mspabi_srll_\n
__mspabi_srll_\n:
	CLRC
	RRC.W	R13
	RRC.W	R12
	.endm

	_srll	15
	_srll	14
	_srll	13
	_srll	12
	_srll	11
	_srll	10
	_srll	9
	_srll	8
	_srll	7
	_srll	6
	_srll	5
	_srll	4
	_srll	3
	_srll	2
	_srll	1
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif

1:	ADD.W	#-1,R14
	CLRC
	RRC.W	R13
	RRC.W	R12
	.global	__mspabi_srll
__mspabi_srll:
	CMP	#0,R14
	JNZ	1b
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif
