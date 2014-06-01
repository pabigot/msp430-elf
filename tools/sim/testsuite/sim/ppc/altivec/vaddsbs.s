# PSIM altivec vaddsbs testcase
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
	.byte 18, -19, 20, -21
	.byte 22, -23, 24, -25
b_foo:	# v3
	.byte -10, 11, -12, 13
	.byte -14, 15, -16, 17
	.byte -18, 19, -20, 21
	.byte -22, 23, -24, 25
c_foo:	# v4
	.byte -110, 111, -112, 113
	.byte -114, 115, -116, 117
	.byte -118, 119, -120, 121
	.byte -122, 123, -124, 125
d1_foo:		# a + b (v5)
	.byte 0, 0, 0, 0
	.byte 0, 0, 0, 0
	.byte 0, 0, 0, 0
	.byte 0, 0, 0, 0
d2_foo:		# a + a (v6)
	.byte 20, -22, 24, -26
	.byte 28, -30, 32, -34
	.byte 36, -38, 40, -42
	.byte 44, -46, 48, -50
d3_foo:		# a + c (v7)
	.byte -100, 100, -100, 100
	.byte -100, 100, -100, 100
	.byte -100, 100, -100, 100
	.byte -100, 100, -100, 100
d4_foo:		# b + c (v8)
	.byte -120, 122, -124, 126
	.byte -128, 127, -128, 127
	.byte -128, 127, -128, 127
	.byte -128, 127, -128, 127

	.text
	.global vaddsbs
vaddsbs:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v4, %r3, c_foo
	load_vr		%v5, %r3, d1_foo
	load_vr		%v6, %r3, d2_foo
	load_vr		%v7, %r3, d3_foo
	load_vr		%v8, %r3, d4_foo

	vaddsbs		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		4, 24, fail

	vaddsbs		%v9, %v2, %v2
	vcmpequb.	%v10, %v9, %v6
	bc		4, 24, fail

	vaddsbs		%v9, %v2, %v4
	vcmpequb.	%v10, %v9, %v7
	bc		4, 24, fail

	vaddsbs		%v9, %v3, %v4
	vcmpequb.	%v10, %v9, %v8
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
