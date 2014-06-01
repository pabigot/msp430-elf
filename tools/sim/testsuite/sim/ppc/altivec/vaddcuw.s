# PSIM altivec vaddcuw testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:
	.long 0x80000000
	.long 0x7fffffff
	.long 0x7fffffff
	.long 0x7fffffff
b_foo:
	.long 0x80000000
	.long 0x7fffffff
	.long 0x80000000
	.long 0x80000001
d_foo:
	.long 0x00000001
	.long 0x00000000
	.long 0x00000000
	.long 0x00000001

	.text
	.global vaddcuw
vaddcuw:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo
	vaddcuw		%v4, %v2, %v3
	vcmpequw.	%v6, %v4, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
