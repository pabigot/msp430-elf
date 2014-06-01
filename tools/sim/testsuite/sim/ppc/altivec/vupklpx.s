# PSIM altivec vupklpx testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.short 0, 0, 0, 0
	.short 0xface, 0x1234, 0x5678, 0x90db

d_foo:	# v5
	.long 0xff1e160e, 0x41114, 0x151318, 0xff04061b

	.text
	.global vupklpx
vupklpx:
	load_vr		%v2, %r3, a_foo
	load_vr		%v5, %r3, d_foo

	vupklpx		%v9, %v2
	vcmpequw.	%v10, %v9, %v5
	bc		12, 24, pass

	fail

pass:
	pass
