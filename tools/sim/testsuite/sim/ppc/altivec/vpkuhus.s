# PSIM altivec vpkuhus testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.short 1, 2, 3, 4
	.short 5, 6, 7, 8
b_foo:	# v3
	.short 254, 255, 256, 257
	.short 258, 259, 260, 261
d_foo:	# v5
	.byte 1, 2, 3, 4
	.byte 5, 6, 7, 8
	.byte 254, 255, 255, 255
	.byte 255, 255, 255, 255

	.text
	.global vpkuhus
vpkuhus:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vpkuhus		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
