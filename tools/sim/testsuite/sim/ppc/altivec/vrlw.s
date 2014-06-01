# PSIM altivec vrlw testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.long 23456, 7654321, 97531, 2468
b_foo:	# v3
	.long 30, 27, 4, 2
d_foo:	# v5
	.long 5864, -2013026723, 1560496, 9872

	.text
	.global vrlw
vrlw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vrlw		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
