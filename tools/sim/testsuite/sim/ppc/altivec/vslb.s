# PSIM altivec vslb testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.long 0xf08f1248, 0x12345678, 0x9abcdef0, 0xffff0000
b_foo:	# v3
	.long 0x01020304, 0x05060708, 0x08070605, 0x04030201
d_foo:	# v5
	.long 0xe03c9080, 0x40000078, 0x9a008000, 0xf0f80000

	.text
	.global vslb
vslb:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vslb		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
