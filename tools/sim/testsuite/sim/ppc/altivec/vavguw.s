# PSIM altivec vavguw testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.long  32014,  32015,  32016,  32017
b_foo:	# v3
	.long     23,     22,     25,     24
d_foo:	# v5
	.long  16019,  16019,  16021,  16021

	.text
	.global vavguw
vavguw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vavguw		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
