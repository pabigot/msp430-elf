/*
 * reloc.S -- FR-V PIC support
 *
 * Copyright (c) 1999, 2000 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
#include <bsp/cpu.h>

        .file "reloc.S"

	.data
	/* statically store .Lcall's address so we can see if we are running
	   at the location we were linked for or a different location.  */
	.type	SYM_NAME(__start_cmp),@object
	.size	SYM_NAME(__start_cmp),4
	.p2align 2
SYM_NAME(__start_cmp):
	.picptr	 .Lcall

	.text

FUNC_START __do_init
	addi	sp,-4,sp
	movsg	lr,gr4
	st	gr4,@(sp,gr0)
	
	call	.Lcall
    .Lcall:
	movsg	lr,gr4
	sethi	#gprelhi(.Lcall),gr5
	setlo	#gprello(.Lcall),gr5
	sub	gr4,gr5,gr16
	
	sethi	#gprelhi(SYM_NAME(__start_cmp)),gr5
	setlo	#gprello(SYM_NAME(__start_cmp)),gr5
	ld	@(gr5,gr16),gr6
	subcc	gr4,gr6,gr8,icc0
	beq	icc0,0,9f
	
	/* fixup the .ctors list */
	sethi	#gprelhi(SYM_NAME(__CTOR_LIST__)),gr9
	sethi	#gprelhi(SYM_NAME(__CTOR_END__)),gr10
	setlo	#gprello(SYM_NAME(__CTOR_LIST__)),gr9
	setlo	#gprello(SYM_NAME(__CTOR_END__)),gr10
	add	gr9,gr16,gr9
	add	gr10,gr16,gr10
	addi	gr9,4,gr9
	subi	gr10,4,gr10
	setlos	4,gr11
	call	SYM_NAME(__frv_fixptrs)

	/* fixup the .dtors list */
	sethi	#gprelhi(SYM_NAME(__DTOR_LIST__)),gr9
	sethi	#gprelhi(SYM_NAME(__DTOR_END__)),gr10
	setlo	#gprello(SYM_NAME(__DTOR_LIST__)),gr9
	setlo	#gprello(SYM_NAME(__DTOR_END__)),gr10
	add	gr9,gr16,gr9
	add	gr10,gr16,gr10
	addi	gr9,4,gr9
	subi	gr10,4,gr10
	call	SYM_NAME(__frv_fixptrs)

	/* fixup the .dtors list */
	sethi	#gprelhi(SYM_NAME(__ROFIXUP_LIST__)),gr9
	sethi	#gprelhi(SYM_NAME(__ROFIXUP_END__)),gr10
	setlo	#gprello(SYM_NAME(__ROFIXUP_LIST__)),gr9
	setlo	#gprello(SYM_NAME(__ROFIXUP_END__)),gr10
	add	gr9,gr16,gr9
	add	gr10,gr16,gr10
	call	SYM_NAME(__frv_fix_usrptrs)
    9:
	ld	@(sp,gr0),gr4
	addi	sp,4,sp
	movgs	gr4,lr
	ret
FUNC_END __do_init


	/* Routine to adjust pointers
	   gr8  = difference to adjust by
	   gr9  = starting address
	   gr10 = ending address + 4
	   gr11 = amount to add to the pointer each iteration.  */
FUNC_START __frv_fixptrs
	sub	gr9,gr11,gr9
	sub	gr10,gr11,gr10
    1:
	cmp	gr10,gr9,icc0
	bls	icc0,0,9f

	ldu	@(gr9,gr11),gr5
	add	gr8,gr5,gr5
	st	gr5,@(gr9,gr0)
	bra	1b
    9:
	ret
FUNC_END __frv_fixptrs

	/* Routine to adjust statically initialized pointers
	   Note since these are pointers to pointers, they
	   need to be adjusted themsevles.

	   gr8  = difference to adjust by
	   gr9  = starting address
	   gr10 = ending address + 4
	   gr11 = amount to add to the pointer each iteration.  */
FUNC_START __frv_fix_usrptrs
	sub	gr9,gr11,gr9
	sub	gr10,gr11,gr10
    1:
	cmp	gr10,gr9,icc0
	bls	icc0,0,9f

	ldu	@(gr9,gr11),gr5
	ld	@(gr5, gr8),gr6
	cmp	gr6,gr0,icc0		/* skip pointers initialized to 0 */
	beq	icc0,0,1b

	add	gr8, gr6, gr6
	st	gr6, @(gr5,gr8)
	bra	1b
    9:
	ret
FUNC_END __frv_fix_usrptrs
