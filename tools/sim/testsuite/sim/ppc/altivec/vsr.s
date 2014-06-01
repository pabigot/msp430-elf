# PSIM altivec vsr testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.long 0xf08f1248, 0x12345678, 0x9abcdef0, 0xffff0000
b_foo:	# v3
	.long 0x04040404, 0x04040404, 0x04040404, 0x04040404
d_foo:	# v5
	.long 0x0f08f124, 0x81234567, 0x89abcdef, 0x0ffff000

	.text
	.global vsr
vsr:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsr		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
