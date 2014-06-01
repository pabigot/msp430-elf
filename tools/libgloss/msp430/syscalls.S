/* Copyright (c) 2012, 2013 Red Hat Incorporated.
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

/* Be wary: the lower N bits of the *address* of the function
   determines the syscall used by the simulator.  Thus, the addresses
   listed here depend on the syscall numbers in ../syscalls.h.  */

/* As per the MSP430x200 Family Users Guide, section 1.5, "An
   instruction fetch from the address range 0x0000 - 0x01FF will reset
   the device."  We take advantage of that to do syscalls in the
   simulator, by trying to execute specific addresses in that range
   and letting the simulator catch them while simulating the CALL
   instruction.  In theory, this is an operation that the physical
   hardware will never attempt to do, so it won't interfere with the
   simulation's accuracy (i.e. we aren't abusing holes in the opcode
   map, for example).  */

#include "../syscall.h"
#include "memmodel.h"
	
.macro	sc,a
	sc2 \a,\a
.endm
	
.macro	sc2,name,num
	.weak	\name
	.global	\name
	\name = 0x180 + \num
.endm

#define SC(n) sc2 n,SYS_##n

	sc2	_exit,SYS_exit
	SC (exit)
	SC (open)
	SC (close)
	SC (read)
/*	SC (write)*/
	SC (lseek)
	SC (unlink)
	SC (kill)
	SC (fstat)

	.weak	isatty
	.global	isatty
isatty:
	.weak	_isatty
	.global	_isatty
_isatty:
	MOV	#1,R12
	ret_
	
	.weak	getpid
	.global	getpid
getpid:
	MOV	#42,R12
	ret_
