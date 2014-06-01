# PSIM altivec vcmpgtub testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte 110, 111, 112, 113
	.byte 114, 115, 116, 117
	.byte 118, 119, 120, 121
	.byte 122, 123, 124, 125
b_foo:	# v3
	.byte 10, 22, 12, 23
	.byte 14, 25, 16, 27
	.byte 18, 29, 20, 31
	.byte 22, 33, 24, 35

	.text
	.global vcmpgtub
vcmpgtub:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo

	vcmpgtub.	%v10, %v2, %v3
	bc		12, 24, pass

fail:
	fail

pass:
	pass
