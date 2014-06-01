# PSIM altivec vsumsws testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.long 5000, 234556342, 2047483642, -40000000
b_foo:	# v3
	.long -2147483640, 2147483641, 2147483642, 94561336
d_foo:	# v5
	.long 0, 0, 0, 2147483647

	.text
	.global vsumsws
vsumsws:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsumsws		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
