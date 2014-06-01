# PSIM altivec vor/vnor/vxor testcase
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
	.long 0x00378421, 0x87654321, 0x0fedcba9, 0x0000ffff
dor_foo:	# v5 (or)
	.long 0xf0bf9669, 0x97755779, 0x9ffddff9, 0xffffffff
dnor_foo:	# v6 (nor)
	.long 0x0f406996, 0x688aa886, 0x60022006, 0
dxor_foo:	# v7 (xor)
	.long 0xf0b89669, 0x95511559, 0x95511559, 0xffffffff

	.text
	.global vor
vor:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, dor_foo
	load_vr		%v6, %r3, dnor_foo
	load_vr		%v7, %r3, dxor_foo

	vor		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		4, 24, fail

	vnor		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v6
	bc		4, 24, fail

	vxor		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v7
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
