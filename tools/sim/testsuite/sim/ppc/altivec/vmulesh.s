# PSIM altivec vmulesh testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.short 10, -11, 12, -13
	.short 14, -15, 16, -17
b_foo:	# v3
	.short -10, 11, -12, 13
	.short -14, 15, -16, 17
d_foo:	# v5
	.long -100, -144, -196, -256

	.text
	.global vmulesh
vmulesh:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vmulesh		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
