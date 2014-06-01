# PSIM altivec vsum2sws testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.long 1, 2, 3, 4
	#.long 5000, 234556342, 2047483642, -40000000
b_foo:	# v3
	#.long 55, 247483642, 55, 94561336
	.long 5, 6, 7, 8
d_foo:	# v5
	#.long 0, 482044984, 0, 2102044978
	.long 0, 9, 0, 15

	.text
	.global vsum2sws
vsum2sws:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsum2sws	%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
