# PSIM altivec vsldoi testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

# BROKEN on the sim

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.long 0xf08f1248, 0x12345678, 0x9abcdef0, 0xffff0000
b_foo:	# v3
	.long 0x01020304, 0x05060708, 0x08070605, 0x04030201
d_foo:	# v5
	.long 0xffff0000, 0x01020304, 0x05060708, 0x08070605

	.text
	.global vsldoi
vsldoi:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsldoi		%v9, %v2, %v3, 12
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
