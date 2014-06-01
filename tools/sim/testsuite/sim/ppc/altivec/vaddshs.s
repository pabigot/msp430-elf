# PSIM altivec vaddshs testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.short 32020, -32021, 32022, -32023
	.short 32024, -32025, 32026, -32027
b_foo:	# v3
	.short -32020, 32021, -32022, 32023
	.short -32024, 32025, -32026, 32027
c_foo:	# v4
	.short -740, 741, -742, 743
	.short -744, 745, -746, 747

d1_foo:		# a + b (v5)
	.short 0, 0, 0, 0
	.short 0, 0, 0, 0
d2_foo:		# a + a (v6)
	.short 32767, -32768, 32767, -32768
	.short 32767, -32768, 32767, -32768
d3_foo:		# a + c (v7)
	.short 31280, -31280, 31280, -31280
	.short 31280, -31280, 31280, -31280
d4_foo:		# b + c (v8)
	.short -32760, 32762, -32764, 32766
	.short -32768, 32767, -32768, 32767

	.text
	.global vaddshs
vaddshs:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v4, %r3, c_foo
	load_vr		%v5, %r3, d1_foo
	load_vr		%v6, %r3, d2_foo
	load_vr		%v7, %r3, d3_foo
	load_vr		%v8, %r3, d4_foo

	vaddshs		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		4, 24, fail

	vaddshs		%v9, %v2, %v2
	vcmpequh.	%v10, %v9, %v6
	bc		4, 24, fail

	vaddshs		%v9, %v2, %v4
	vcmpequh.	%v10, %v9, %v7
	bc		4, 24, fail

	vaddshs		%v9, %v3, %v4
	vcmpequh.	%v10, %v9, %v8
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
