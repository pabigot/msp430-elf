# PSIM altivec vmrghb testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte	0,  1,  2,  3
	.byte	4,  5,  6,  7
	.byte   8,  9,  10, 11
	.byte   12, 13, 14, 15
b_foo:	# v3
	.byte	16, 17, 18, 19
	.byte	20, 21, 22, 23
	.byte	24, 25, 26, 27
	.byte	28, 29, 30, 31
d_foo:	# v5
	.byte   0,  16, 1,  17
	.byte	2,  18, 3,  19
	.byte   4,  20, 5,  21
	.byte	6,  22, 7,  23

	.text
	.global vmrghb
vmrghb:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vmrghb		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
