# PSIM altivec vsububm testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte 245, 246, 247, 248
	.byte 249, 250, 251, 252
	.byte 253, 254, 255, 254
	.byte 253, 252, 251, 250
b_foo:	# v3
	.byte 10, 11, 12, 13
	.byte 14, 15, 16, 17
	.byte 18, 19,  0,  2
	.byte  4,  6,  8, 10
d_foo:	# v5
	.byte 235, 235, 235, 235
	.byte 235, 235, 235, 235
	.byte 235, 235, 255, 252
	.byte 249, 246, 243, 240

	.text
	.global vsububm
vsububm:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsububm		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
