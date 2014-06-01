# PSIM altivec vmsumshm testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.short	8, -9
	.short	0, -1
	.short	-4, -5
	.short	6, -7
b_foo:	# v3
	.short	5,  3
	.short	2, -3
	.short	-7, -8
	.short	-6,  7
c_foo:	# v4
	.long	10, 20, 30, 40
d_foo:	# v5
	.long	23, 23, 98, -45

	.text
	.global vmsumshm
vmsumshm:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v4, %r3, c_foo
	load_vr		%v5, %r3, d_foo

	vmsumshm	%v9, %v2, %v3, %v4
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
