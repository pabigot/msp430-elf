# PSIM altivec vmsummbm testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte	8, -9, 0, -1
	.byte	4, -5, 6, -7
	.byte	1, -1, 2, -3
	.byte	2, -3, 4, -5
b_foo:	# v3
	.byte	5, 3, 2, 3
	.byte	7, 8, 6, 7
	.byte	9, 1, 0, 1
	.byte	5, 3, 4, 5
c_foo:	# v4
	.long	10, 20, 30, 40
d_foo:	# v5 
	.long	20, -5, 35, 32

	.text
	.global vmsummbm
vmsummbm:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v4, %r3, c_foo
	load_vr		%v5, %r3, d_foo

	vmsummbm	%v9, %v2, %v3, %v4
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
