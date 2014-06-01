# PSIM altivec vsubcuw testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:
	.long 10
	.long 50
	.long 128
	.long 0
b_foo:
	.long 9
	.long 45
	.long 256
	.long 6
d_foo:
	.long 1
	.long 1
	.long 0
	.long 0

	.text
	.global vsubcuw
vsubcuw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo
	vsubcuw		%v4, %v2, %v3
	vcmpequw.	%v6, %v4, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
