.text
.align 0
	# New basic instructions.
	LDL.W   [r24], r25
	STC.W   r26, [r27]
	BINS    r28, 0x2, 0x3, r29
	ROTL    0x4, r5, r6
	ROTL    r7, r8, r9
	LD.DW   0x723456[r10], r12
	ST.DW 	r12, 0x123456[r13]

	# New floating point instructions.
	CVTF.HS r8, r9
	CVTF.SH r10, r11
	FMAF.S  r12, r13, r14
	FMSF.S  r15, r16, r17
	FNMAF.S r18, r19, r20
	FNMSF.S r21, r22, r23

	# New cache instructions.
	PREF  prefi, [r14]
	PREF  prefd, [r15]

	CACHE  cfald,   [r16]
	CACHE  cfali,	[r17]
	CACHE  chbid,	[r18]
	CACHE  chbii,	[r19]
	CACHE  chbiwbd,	[r20]
	CACHE  chbwbd,	[r21]
	CACHE  cibid,	[r22]
	CACHE  cibii,	[r23]
	CACHE  cibiwbd,	[r24]
	CACHE  cibwbd,	[r25]
	CACHE  cildd,	[r26]
	CACHE  cildi,	[r27]
	CACHE  cistd,	[r28]
	CACHE  cisti,	[r29]
