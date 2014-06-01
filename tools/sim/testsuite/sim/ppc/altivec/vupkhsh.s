# PSIM altivec vupkhsh testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.short	-1, -5, -8, 3
	.short	0, 0, 0, 0
d_foo:	# v5
	.long	-1, -5, -8, 3

	.text
	.global vupkhsh
vupkhsh:
	load_vr		%v2, %r3, a_foo
	load_vr		%v5, %r3, d_foo

	vupkhsh		%v9, %v2
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
