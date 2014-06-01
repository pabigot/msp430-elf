# PSIM altivec vupkhsb testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.byte	-1, -5, -8, 3
	.byte	-124, 4, 3, 23
	.byte	0, 0, 0, 0
	.byte	0, 0, 0, 0
d_foo:	# v5
	.short	-1, -5, -8, 3
	.short	-124, 4, 3, 23

	.text
	.global vupkhsb
vupkhsb:
	load_vr		%v2, %r3, a_foo
	load_vr		%v5, %r3, d_foo

	vupkhsb		%v9, %v2
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
