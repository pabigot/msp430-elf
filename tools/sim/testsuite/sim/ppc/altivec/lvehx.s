# PSIM altivec lvehx testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
ld_foo:
	.short 4
	.short 5
	.short 6
	.short 7
	.short 8
	.short 9
	.short 10
	.short 11

	.text
	.global lvehx
lvehx:
	lis		%r3, ld_foo@ha
	addi		%r3, %r3, ld_foo@l
	li		%r4, 6
	lvehx		%v1, %r3, %r4
	vsplth		%v2, %v1, 3
	vspltish	%v3, 7
	vcmpequh.	%v4, %v2, %v3
	bc		12, 24, pass
fail:
	fail

pass:
	pass
