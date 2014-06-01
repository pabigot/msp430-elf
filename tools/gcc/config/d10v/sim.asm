/*
 * special support for simulator
 *
 *   Copyright (C) 1996, 2009 Free Software Foundation, Inc.
 *   Written By Michael Meissner
 * 
 * This file is free software.  You can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation * either version 3, or (at your option) any
 * later version.
 * 
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY * without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with this program;
 * see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

	.file	"sim.s"
	.text

	.globl	printf
	.type	printf,@function
printf:
	trap	2
	jmp	r13
	.size	printf,.-printf

	.globl	putchar
	.type	putchar,@function
putchar:
	trap	3
	jmp	r13
	.size	putchar,.-putchar

	.globl	putstr
	.type	putstr,@function
putstr:
	trap	1
	jmp	r13
	.size	putstr,.-putstr

	.globl	exit
	.type	exit,@function
	.globl	_exit
	.type	_exit,@function
exit:
_exit:
	ldi	r6,1 -> trap 0			/* SYS_exit */
	stop
	.size	exit,.-exit
	.size	_exit,.-_exit

	.globl	abort
	.type	aobrt,@function
abort:
	ldi	r2,amsg
	trap	1
	ldi	r6,1 || ldi r2,1		/* SYS_exit & non-zero return code */
	trap	0 -> stop

	.section .rodata
amsg:	.ascii	"Abort called\n\0"
