# PSIM altivec vslw testcase
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
	.long 0x00000004, 0x00000008, 0x00000005, 0x00000001
d_foo:	# v5
	.long 0x08f12480, 0x34567800, 0x579bde00, 0xfffe0000

	.text
	.global vslw
vslw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vslw		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
