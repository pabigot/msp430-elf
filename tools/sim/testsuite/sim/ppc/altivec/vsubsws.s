# PSIM altivec vsubsws testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.long 2147483640, -2147483641, 2147483642, -2147483643
b_foo:	# v3
	.long -2147483640, 2147483641, 2147483642, 2147483643
d_foo:	# v5
	.long 2147483647, -2147483648, 0, -2147483648

	.text
	.global vsubsws
vsubsws:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsubsws		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
