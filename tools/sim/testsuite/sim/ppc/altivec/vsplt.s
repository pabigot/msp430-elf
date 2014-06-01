# PSIM altivec vspltisb testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte -2, -2, -2, -2
	.byte -2, -2, -2, -2
	.byte -2, -2, -2, -2
	.byte -2, -2, -2, -2
b_foo:	# v4
	.short -2, -2, -2, -2
	.short -2, -2, -2, -2
c_foo:	# v5
	.long -2, -2, -2, -2

	.text
	.global vrlw
vrlw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v4, %r3, b_foo
	load_vr		%v5, %r3, c_foo

	vspltisb	%v9, -2
	vcmpequb.	%v10, %v9, %v2
	bc		4, 24, fail

	vspltish	%v9, -2
	vcmpequh.	%v10, %v9, %v4
	bc		4, 24, fail

	vspltisw	%v9, -2
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass


fail:
	fail

pass:
	pass
