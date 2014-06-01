# PSIM altivec vsubuhm testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.short 32820, 32021, 32822, 32023
	.short 32024, 32025, 32026, 32927
b_foo:	# v3
	.short 32024, 32025, 32026, 32927
	.short 32820, 32021, 32822, 32023
d_foo:	# v5
	.short 796, 65532, 796, 64632
	.short 64740, 4, 64740, 904

	.text
	.global vsubuhm
vsubuhm:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsubuhm		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
