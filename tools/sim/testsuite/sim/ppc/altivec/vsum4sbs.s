# PSIM altivec vsum4sbs testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.byte 50, 40, 30, 20	# 140
	.byte 100, 110, 120, 90 # 420
	.byte 100, 101, 102, 103 # 406
	.byte 8, 6, 4, 2 # 20
b_foo:	# v3
	.long 14, 23, 42, 81
d_foo:	# v5
	.long 154, 443, 448, 101

	.text
	.global vsum4sbs
vsum4sbs:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vsum4sbs		%v9, %v2, %v3
	vcmpequw.	%v10, %v9, %v5
	bc		4, 24, fail

	b		pass
fail:
	fail

pass:
	pass
