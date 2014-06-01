# PSIM altivec vaddsws testcase
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
	.long -2147483640, 2147483641, -2147483642, 2147483643
c_foo:	# v4
	.long -6, 7, -8, 9

d1_foo:		# a + b (v5)
	.long 0, 0, 0, 0
d2_foo:		# a + a (v6)
	.long 2147483647, -2147483648, 2147483647, -2147483648
d3_foo:		# a + c (v7)
	.long 2147483634, -2147483634, 2147483634, -2147483634
d4_foo:		# b + c (v8)
	.long -2147483646, 2147483647, -2147483648, 2147483647

	.text
	.global vaddsws
vaddsws:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v4, %r3, c_foo
	load_vr		%v5, %r3, d1_foo
	load_vr		%v6, %r3, d2_foo
	load_vr		%v7, %r3, d3_foo
	load_vr		%v8, %r3, d4_foo

	vaddsws		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		4, 24, fail

	vaddsws		%v9, %v2, %v2
	vcmpequw.	%v10, %v9, %v6
	bc		4, 24, fail

	vaddsws		%v9, %v2, %v4
	vcmpequw.	%v10, %v9, %v7
	bc		4, 24, fail

	vaddsws		%v9, %v3, %v4
	vcmpequw.	%v10, %v9, %v8
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
