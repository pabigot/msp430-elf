# PSIM altivec vpkswss testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.long 1, 2, 3, 4
b_foo:	# v3
	.long 32767, 32768, -32768, -32769
d_foo:	# v5
	.short 1, 2, 3, 4
	.short 32767, 32767, -32768, -32768

	.text
	.global vpkswss
vpkswss:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vpkswss		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
