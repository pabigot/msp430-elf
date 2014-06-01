/*
 * special crt0 support for simulator
 *
 *   Copyright (C) 1996, 1997, 1998, 2009 Free Software Foundation, Inc.
 *   Written By Michael Meissner
 * 
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3, or (at your option) any
 * later version.
 * 
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
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
 * <http://www.gnu.org/licenses/>.  */

	.file	"scrt0.s"
	.text
	.type	_start,@function
	.globl	_start
_start:	ldi	r14,0
	ldi	sp, _stack
	bl	main
	stop
	.size	_start,.-_start
