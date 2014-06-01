# PSIM altivec lvebx testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
ld_foo:
	.byte 42
	.byte 43
	.byte 44
	.byte 45
	.byte 46
	.byte 13
	.byte 48
	.byte 49
	.byte 42
	.byte 43
	.byte 44
	.byte 45
	.byte 46
	.byte 13
	.byte 48
	.byte 49

	.text
	.global lvebx
lvebx:
	lis		%r3, ld_foo@ha
	addi		%r3, %r3, ld_foo@l
	li		%r4, 5
	lvebx		%v1, %r3, %r4
	vspltb		%v2, %v1, 5
	vspltisb	%v3, 13
	vcmpequb.	%v4, %v2, %v3
	bc		12, 24, pass
fail:
	fail

pass:
	pass
