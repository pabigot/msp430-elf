# PSIM altivec vcmpgtuw testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.long 150000000, 151333000, 152023400, 51302300
b_foo:	# v3
	.long 10000000, 22333000, 12023400, 2302300

	.text
	.global vcmpgtuw
vcmpgtuw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo

	vcmpgtuw.	%v10, %v2, %v3
	bc		12, 24, pass

fail:
	fail

pass:
	pass
