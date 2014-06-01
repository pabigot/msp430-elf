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

	.global	__mspabi_func_epilog_7
	.global	__mspabi_func_epilog_6
	.global	__mspabi_func_epilog_5
	.global	__mspabi_func_epilog_4
	.global	__mspabi_func_epilog_3
	.global	__mspabi_func_epilog_2
	.global	__mspabi_func_epilog_1

__mspabi_func_epilog_7:
	POP	R4
__mspabi_func_epilog_6:
	POP	R5
__mspabi_func_epilog_5:
	POP	R6
__mspabi_func_epilog_4:
	POP	R7
__mspabi_func_epilog_3:
	POP	R8
__mspabi_func_epilog_2:
	POP	R9
__mspabi_func_epilog_1:
	POP	R10
#ifdef __MSP430X_LARGE__
	RETA
#else
	RET
#endif
