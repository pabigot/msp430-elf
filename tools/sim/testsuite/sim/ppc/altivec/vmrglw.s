# PSIM altivec vmrglw testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.long	0,  1,  2,  3
b_foo:	# v3
	.long	16, 17, 18, 19
d_foo:	# v5
	.long	2,  18, 3,  19

	.text
	.global vmrglw
vmrglw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vmrglw		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
