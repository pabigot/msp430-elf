# PSIM altivec vsum4shs testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.short 50, 40, 30, 20
	.short 100, 110, 120, 130
b_foo:	# v3
	.long 14, 23, 42, 81
d_foo:	# v5
	.long 104, 73, 252, 331

	.text
	.global vsum4shs
vsum4shs:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsum4shs		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
