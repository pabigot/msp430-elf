# PSIM altivec vminsw testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.long     10,    -11,     12,    -13
b_foo:	# v3
	.long -32018,  32019, -32020,  32021
d_foo:	# v5
	.long -32018,    -11, -32020,    -13

	.text
	.global vminsw
vminsw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vminsw		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
