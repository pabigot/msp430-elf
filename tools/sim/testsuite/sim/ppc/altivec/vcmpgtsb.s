# PSIM altivec vcmpgtsb testcase
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
	.byte -10,-22, -12,-23
	.byte -14,-25, -16,-27
	.byte -18,-29, -20,-31
	.byte -22,-33, -24,-35

	.text
	.global vcmpgtsb
vcmpgtsb:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo

	vcmpgtsb.	%v10, %v2, %v3
	bc		12, 24, pass

fail:
	fail

pass:
	pass
