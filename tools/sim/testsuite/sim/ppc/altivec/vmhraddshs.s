# PSIM altivec vmhraddshs testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.short   8000,  -4000,  18384, -24590
	.short  16024, -16025,    -32,  13191
b_foo:	# v3
	.short     -3,      3,     -2,      2
	.short     -1,      1,     -4,      4
c_foo:	# v4
	.short    -20,     21,    -22,     23
	.short    -24,     25,    -26,     27
d_foo:	# v5
	.short    -21,     21,    -23,     21
	.short    -24,     25,    -26,     29
	#.short  -7636,   4405, -16406, -23593
	#.short    336,    384,  16486,  32767


	.text
	.global vmhraddshs
vmhraddshs:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v4, %r3, c_foo
	load_vr		%v5, %r3, d_foo

	vmhraddshs	%v9, %v2, %v3, %v4
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
