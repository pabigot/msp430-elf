	.file "strcpy.S"

	.section .text
#if 0
	.global _strcpy
	.type	 _strcpy,@function
_strcpy:
	movm [d2,d3,a2,a3],(sp)
.Lend_of_prologue:
	mov d0,d3
	mov d1,a0
	mov d3,a1
	mov a0,d0
#ifndef __OPTIMIZE_SIZE__
	or d3,d0
	btst 3,d0
	bne .L2
	mov (a0),d0
	mov -16843009,a2
	mov a2,d1
	add d0,d1
	not d0
	and d0,d1
	mov -2139062144,d2
	btst -2139062144,d1
	bne .L2
	setlb
	mov (a0),d0
	mov d0,(a1)
	inc4 a0
	inc4 a1
	mov (a0),d0
	mov a2,d1
	add d0,d1
	not d0
	and d0,d1
	and d2,d1
	leq
.L2:
#endif	
	setlb
	movbu (a0),d0
	movbu d0,(a1)
	inc a0
	inc a1
	cmp 0,d0
	lne
	mov d3,a0
#else
	.global  _strcpy
	.type	 _strcpy,@function
_strcpy:
	movm	[d2,d3,a3],(sp)
.Lend_of_prologue:
	mov	d0, d3
	mov	d1, a0
	mov	d3, a1
	mov	a0, d0

/*	Newlib for the MN10300 is compiled with -Os enabled all the time,
	so normally the code below will not be triggered.  The test for
	__LIW__ however enables us to build a speed optimized version when
	building -mliw enabled multilibs.  -mliw allows long instruction
	word generation which is faster but occupies more space.  LIW
	instructions are not acutally used in the code below, but enabling
	their generation implies that the user is more interested in code
	performance than code size.  */
#if ! defined __OPTIMIZE_SIZE__ || defined __LIW__
	or	d3, d0
	btst	3, d0
	bne	.L2
	mov	(a0), d0
	mov	-16843009, a3
	mov	a3, d1
	add	d0, d1
	not	d0
	and	d0, d1
	mov	-2139062144, d2
	btst	-2139062144, d1
	bne	.L2
#ifdef __NO_SETLB__
.Loop1:
#else
	setlb
#endif
	mov	(a0), d0
	mov	d0, (a1)
	inc4	a0
	inc4	a1
	mov	(a0), d0
	mov	a3, d1
	add	d0, d1
	not	d0
	and	d0, d1
	and	d2, d1
#ifdef __NO_SETLB__
	beq    .Loop1
#else
	leq
#endif
.L2:
#endif /* Optimize for speed.  */

#ifdef __NO_SETLB__
.Loop2:
#else
	setlb
#endif
	movbu	(a0), d0
	movbu	d0, (a1)
	inc	a0
	inc	a1
	cmp	0, d0
#ifdef __NO_SETLB__
	bne	.Loop2
#else
	lne
#endif

	mov	d3, a0
	mov	d3, d0
	ret	[d2,d3,a3], 12
#endif
.Lepilogue:
	ret [d2,d3,a2,a3],16
.Lend_of_strcpy:
	.size	 _strcpy, .Lend_of_strcpy - _strcpy

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
	.4byte	 _strcpy			# FDE initial location
	.4byte	 .Lend_of_strcpy - _strcpy	# FDE address range
	.byte	 0x4				# DW_CFA_advance_loc4
	.4byte	 .Lend_of_prologue - _strcpy
	.byte	 0xe				# DW_CFA_def_cfa_offset
	.uleb128 0x4
	.byte	 0x87				# DW_CFA_offset, column 0x7
	.uleb128 0x1
	.align 2
.Lend_of_FDE:
