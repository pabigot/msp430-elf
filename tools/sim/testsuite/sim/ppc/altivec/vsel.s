# PSIM altivec vsel testcase
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
c_foo:	# v4
	.long 0xfedcba98, 0x76543210, 0x01234567, 0x89abcdef
d_foo:	# v5
	.long 0x00178040, 0x06644668, 0x9bbddbb1, 0x7654cdef

	.text
	.global vsel
vsel:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v4, %r3, c_foo
	load_vr		%v5, %r3, d_foo

	vsel		%v9, %v2, %v3, %v4
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
