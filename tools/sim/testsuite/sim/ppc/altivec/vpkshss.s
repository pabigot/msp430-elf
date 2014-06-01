# PSIM altivec vpkshss testcase
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
	.short 126, 127, 128, 129
	.short -129, -128, -127, -126
d_foo:	# v5
	.byte 1, 2, 3, 4
	.byte 5, 6, 7, 8
	.byte 126, 127, 127, 127
	.byte -128, -128, -127, -126

	.text
	.global vpkshss
vpkshss:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vpkshss		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
