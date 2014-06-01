# PSIM altivec vmaxub testcase
# mach: altivec altivecle
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4
a_foo:	# v2
	.byte  10, 11, 12, 13
	.byte  14, 15, 16, 17
	.byte  18, 19, 20, 21
	.byte  22, 23, 24, 25
b_foo:	# v3
	.byte  18, 19, 20, 21
	.byte  22, 23, 24, 25
	.byte  14, 15, 16, 17
	.byte  10, 11, 12, 13
d_foo:	# v5
	.byte  18, 19, 20, 21
	.byte  22, 23, 24, 25
	.byte  18, 19, 20, 21
	.byte  22, 23, 24, 25

	.text
	.global vmaxub
vmaxub:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vmaxub		%v9, %v2, %v3
	vcmpequb.	%v10, %v9, %v5
	bc		12, 24, pass
fail:
	fail

pass:
	pass
