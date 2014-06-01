# PSIM altivec vmaxsh testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.short     10,    -11,     12,    -13
	.short  32014, -32015,  32016, -32017
b_foo:	# v3
	.short -32018,  32019, -32020,  32021
	.short     23,    -22,     25,    -24
d_foo:	# v5
	.short     10,  32019,     12,  32021
	.short  32014,    -22,  32016,    -24

	.text
	.global vmaxsh
vmaxsh:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vmaxsh		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
