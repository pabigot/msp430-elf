# PSIM altivec vsubuws testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.long 3000000000, 1500000200, 3000000400, 2500000600
b_foo:	# v3
	.long 1000000000, 3000000100, 2000000200, 3000000300
d_foo:	# v5
	.long 2000000000, 0, 1000000200, 0

	.text
	.global vsubuws
vsubuws:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsubuws		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
