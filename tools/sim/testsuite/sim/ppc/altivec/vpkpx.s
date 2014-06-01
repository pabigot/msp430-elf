# PSIM altivec vpkpx testcase
# mach: altivec
# as(altivecle): -mlittle
# ld(altivecle): -EL

	.include "testutils.inc"

	start

	.data
	.p2align 4

a_foo:	# v2
	.byte 1, 16, 24, 32
	.byte 0, 40, 48, 56+1
	.byte 1, 64, 72, 80+2
	.byte 0, 88+3, 96, 104
b_foo:	# v3
	.byte 11, 112, 120, 128+1
	.byte 12, 136+3, 144, 152
	.byte 13, 160, 168, 176
	.byte 14, 184, 192+2, 200
d_foo:	# v5
	.short 34916, 5319, 41258, 11661
	.short 47600, 18003, 53942, 24345

	.text
	.global vpkpx
vpkpx:
	load_vr		%v2, %r3, a_foo
	load_vr		%v3, %r3, b_foo
	load_vr		%v5, %r3, d_foo

	vpkpx		%v9, %v2, %v3
	vcmpequh.	%v10, %v9, %v5
	bc		12, 24, pass

fail:
	fail

pass:
	pass
