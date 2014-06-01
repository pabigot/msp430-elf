/* Copyright (c) 2012-2013 Red Hat Incorporated.
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

#include "memmodel.h"

;; The linker links all .crt_* sections in asciibetical order at the
;; same place.  So, the four digits in .crt_NNNN determine the link
;; order, so, keep them in sequential order here.  The first two
;; digits are set here, the second two allow users to insert code
;; between code fragments here.

#if L0
	.section ".resetvec", "a"
__msp430_resetvec_hook:
	.word	__start

	.section ".crt_0000init", "ax", @progbits
	.refsym	__msp430_resetvec_hook
#ifdef MINRT
	.refsym	__crt0_call_just_main
#else
	.refsym	__crt0_call_init_then_main
#endif
	.global __start
__start:
	mov_	#__stack, R1

	;; Disable watchdog timer.
	MOV	#0x5a80, &0x15C
#endif



;;  Note - if needed sections .crt_bss and/or .crt_movedata
;;  will be inserted here.
#if Lbss
	.section ".crt_0100bss", "ax", @progbits

	.global __crt0_init_bss
__crt0_init_bss:	
	
	mov_	#__bssstart, R12
	clr.w	R13
	mov.w	#__bsssize, R14
#ifdef __MSP430X_LARGE__
	clr.w	R15		; We assume that __bsssize is never > 64M
#endif
	call_	#memset
#endif

#if Lmovedata
	.section ".crt_0200movedata", "ax", @progbits

	.global __crt0_movedata
__crt0_movedata:	
	
	mov_	#__datastart, R12
	mov_	#__romdatastart, R13

	;;  memmove and memcpy do not currently work when src == dst
	cmp_	R12, R13
	jeq	1f

	mov.w	#__romdatacopysize, R14
#ifdef __MSP430X_LARGE__
	clr.w	R15		; We assume that __romdatacopysize is never > 64M
#endif
	call_	#memmove
1:	
#endif


#if Lmain_minrt
	.section ".crt_0300main", "ax", @progbits
	.global	__crt0_call_just_main
__crt0_call_just_main:
	clr.w	R12		; Set argc == 0
	call_	#main
#endif

#if Lmain
	.section ".crt_0300main", "ax", @progbits
	.global	__crt0_call_init_then_main
__crt0_call_init_then_main:
	call_	#__msp430_init

	clr.w	R12		; Set argc == 0
	call_	#main
#endif

#if Lcallexit
	.section ".crt_0400main_exit", "ax", @progbits
	.global	__crt0_call_exit
__crt0_call_exit:
	call_	#_exit
#endif


;----------------------------------------

#ifndef MINRT
#if L0
	.section ".crt_0500main_init", "ax", @progbits
	.global	_msp430_run_init_array
	.type	_msp430_run_init_array,@function
_msp430_run_init_array:
	mov_	#__init_array_start, R4
	mov_	#__init_array_end, R5
	mov_	#PTRsz, R6
	br_	#_msp430_run_array

	.global	_msp430_run_preinit_array
	.type	_msp430_run_preinit_array,@function
_msp430_run_preinit_array:
	mov_	#__preinit_array_start, R4
	mov_	#__preinit_array_end, R5
	mov_	#PTRsz, R6
	br_	#_msp430_run_array

	.global	_msp430_run_fini_array
	.type	_msp430_run_fini_array,@function
_msp430_run_fini_array:
	mov_	#__fini_array_start, R4
	mov_	#__fini_array_end, R5
	mov_	#-PTRsz, R6
	br_	#_msp430_run_array

_msp430_run_array:
	cmp_	R4, R5
	jeq	_msp430_run_done
	mov_	@R4, R7
	add_	R6, R4
	call_	@R7
	br_	_msp430_run_array

_msp430_run_done:
	ret_

;----------------------------------------

	.section	.init,"ax"

	.global __msp430_init
__msp430_init:
	
	.section	.fini,"ax"

	.global __msp430_fini
__msp430_fini:
	call_	#_msp430_run_fini_array
	
#endif
#endif
