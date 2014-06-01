# PSIM altivec lvewx testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
ld_foo:
	.long 4
	.long 5
	.long 6
	.long 7

	.text
	.global lvehx
lvehx:
	lis		%r3, ld_foo@ha
	addi		%r3, %r3, ld_foo@l
	li		%r4, 8
	lvewx		%v1, %r3, %r4
	vspltw		%v2, %v1, 2
	vspltisw	%v3, 6
	vcmpequw.	%v4, %v2, %v3
	bc		12, 24, pass
fail:
	fail

pass:
	pass
