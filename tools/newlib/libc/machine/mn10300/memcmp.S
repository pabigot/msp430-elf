	.file "memcmp.S"

	.section .text
#if 0
	.global _memcmp
	.type	 _memcmp,@function
_memcmp:
	movm [d2,d3,a2,a3],(sp)
.Lend_of_prologue:
	mov d0,a0
	mov d1,a1
	mov (28,sp),a2
#ifndef __OPTIMIZE_SIZE__
	cmp 3,a2
	bls .L22
	mov a1,d2
	or d2,d0
	btst 3,d0
	bne .L22
	setlb
	mov (a0),d1
	mov (a1),d0
	cmp d0,d1
	bne .L22
	inc4 a0
	inc4 a1
	add -4,a2
	cmp 3,a2
	lhi
.L22:
#endif
	cmp 0,a2
	beq .L24
	setlb
	movbu (a0),d3
	movbu (a1),d2
	cmp d2,d3
	beq .L23
	mov d3,d0
	sub d2,d0
	jmp .Lepilogue
.L23:
	inc a0
	inc a1
	sub 1,a2
	lne
.L24:
	clr d0
.Lepilogue:
	ret [d2,d3,a2,a3],16
#else
	.global  _memcmp
	.type	 _memcmp,@function
_memcmp:
	movm  [d2,d3,a3], (sp)
.Lend_of_prologue:
	mov   d0, a0
	mov   d1, a1
	mov   (24, sp), a3

/*	Newlib for the MN10300 is compiled with -Os enabled all the time,
	so normally the code below will not be triggered.  The test for
	__LIW__ however enables us to build a speed optimized version when
	building -mliw enabled multilibs.  -mliw allows long instruction
	word generation which is faster but occupies more space.  LIW
	instructions are not acutally used in the code below, but enabling
	their generation implies that the user is more interested in code
	performance than code size.  */
#if ! defined __OPTIMIZE_SIZE__ || defined __LIW__
	cmp   3, a3
	bls   .L22
	mov   a1, d2
	or    d2, d0
	btst  3, d0
	bne   .L22
#ifdef __NO_SETLB__
.Loop1:
#else
	setlb
#endif
	mov   (a0), d1
	mov   (a1), d0
	cmp   d0, d1
	bne   .L22
	inc4  a0
	inc4  a1
	add   -4, a3
	cmp   3, a3
#ifdef __NO_SETLB__
	bhi   .Loop1
#else
	lhi
#endif
.L22:
#endif /* Optimize for speed.  */

	cmp   0, a3
	beq   .L24
#ifdef __NO_SETLB__
.Loop2:
#else
	setlb
#endif
	movbu (a0), d3
	movbu (a1), d2
	cmp   d2, d3
	beq   .L23
	mov   d3, d0
	sub   d2, d0
	jmp   .Lepilogue
.L23:
	inc   a0
	inc   a1
	sub   1, a3
#ifdef __NO_SETLB__
	bne   .Loop2
#else
	lne
#endif
.L24:
	clr   d0
.Lepilogue:
	ret   [d2,d3,a3], 12
#endif
.Lend_of_func:
	.size	 _memcmp, .Lend_of_func - _memcmp

	.section	.debug_frame,"",@progbits
.Lstart_of_debug_frame:
	# Common Information Entry (CIE)
	.4byte	.Lend_of_CIE - .Lstart_of_CIE	# CIE Length
.Lstart_of_CIE:
	.4byte	 0xffffffff			# CIE Identifier Tag
	.byte	 0x1				# CIE Version
	.ascii   "\0"				# CIE Augmentation
	.uleb128 0x1				# CIE Code Alignment Factor
	.sleb128 -4				# CIE Data Alignment Factor
	.byte	 0x32				# CIE RA Column
	.byte	 0xc				# DW_CFA_def_cfa
	.uleb128 0x9
	.uleb128 0x0
	.byte	 0xb2				# DW_CFA_offset, column 0x32
	.uleb128 0x0
	.align   2
.Lend_of_CIE:
	
	# Frame Description Entry (FDE)
	.4byte	.Lend_of_FDE - .Lstart_of_FDE	# FDE Length
.Lstart_of_FDE:
	.4byte	 .Lstart_of_debug_frame		# FDE CIE offset
	.4byte	 _memcmp			# FDE initial location
	.4byte	 .Lend_of_func - _memcmp	# FDE address range
	.byte	 0x4				# DW_CFA_advance_loc4
	.4byte	 .Lend_of_prologue - _memcmp
	.byte	 0xe				# DW_CFA_def_cfa_offset
	.uleb128 0x4
	.byte	 0x87				# DW_CFA_offset, column 0x7
	.uleb128 0x1
	.align 2
.Lend_of_FDE:
