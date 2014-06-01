# PSIM altivec vmulesb testcase
# mach: altivec
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
d_foo:	# v5
	.short -100, -144, -196, -256
	.short -324, -400, -484, -576

	.text
	.global vmulesb
vmulesb:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vmulesb		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
