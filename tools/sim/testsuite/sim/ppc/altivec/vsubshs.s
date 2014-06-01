# PSIM altivec vsubshs testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.short 32020, -32021, 5, -32023
	.short 32024, -32025, 32026, -32027
b_foo:	# v3
	.short -32020, 32021, -32022, 32023
	.short 32024, 32025, 32026, 32027
d_foo:	# v5
	.short 32767, -32768, 32027, -32768
	.short 0, -32768, 0, -32768

	.text
	.global vsubshs
vsubshs:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsubshs		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
