# PSIM altivec vrlb testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte 2, 6, 4, 8
	.byte 1, 5, 3, 7
	.byte 1, 5, 3, 7
	.byte 2, 6, 4, 8
b_foo:	# v3
	.byte 1, 2, 3, 4
	.byte 5, 6, 7, 8
	.byte 1, 2, 3, 4
	.byte 5, 6, 7, 8
d_foo:	# v5
	.byte 4, 24, 32, 128
	.byte 32, 65, 129, 7
	.byte 2, 20, 24, 112
	.byte 64, 129, 2, 8

	.text
	.global vrlb
vrlb:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vrlb		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
