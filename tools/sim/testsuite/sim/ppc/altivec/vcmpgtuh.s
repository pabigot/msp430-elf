# PSIM altivec vcmpgtuh testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.short 40000, 41000, 42000, 43000
	.short 44000, 45000, 46000, 47000
b_foo:	# v3
	.short 10000, 22000, 12000, 23000
	.short 14000, 25000, 16000, 27000

	.text
	.global vcmpgtuh
vcmpgtuh:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo

	vcmpgtuh.	%v10, %v2, %v3
	bc		12, 24, pass

fail:
	fail

pass:
	pass
