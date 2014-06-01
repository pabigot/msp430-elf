# PSIM altivec vsubsbs testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte 10, -11, 12, -13
	.byte 14, -15, 16, -17
	.byte 18, -19, 125, -21
	.byte 22, -23, 24, -25
b_foo:	# v3
	.byte -10, 11, -12, 13
	.byte -14, 15, -16, 17
	.byte -18, 119, -20, 21
	.byte -22, 23, -24, 25
d_foo:	# v5
	.byte 20, -22, 24, -26
	.byte 28, -30, 32, -34
	.byte 36, -128, 127, -42
	.byte 44, -46, 48, -50

	.text
	.global vsubsbs
vsubsbs:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsubsbs		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
