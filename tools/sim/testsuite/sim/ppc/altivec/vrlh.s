# PSIM altivec vrlh testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.short 2, 6, 10, 12
	.short 1, 5, 9, 13
b_foo:	# v3
	.short 2, 4, 6, 8
	.short 3, 7, 11, 15
d_foo:	# v5
	.short 8, 96, 640, 3072
	.short 8, 640, 18432, 32774

	.text
	.global vrlh
vrlh:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vrlh		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
