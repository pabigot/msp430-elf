	.code
	.align 4
	.level 2.0
; Basic immediate instruction tests.  
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.
	fldw %r4(%sr0,%r5),%fr6
	fldw,s %r4(%sr0,%r5),%fr6
	fldw,m %r4(%sr0,%r5),%fr6
	fldw,sm %r4(%sr0,%r5),%fr6
	fldd %r4(%sr0,%r5),%fr6
	fldd,s %r4(%sr0,%r5),%fr6
	fldd,m %r4(%sr0,%r5),%fr6
	fldd,sm %r4(%sr0,%r5),%fr6
	fstw %fr6,%r4(%sr0,%r5)
	fstw,s %fr6,%r4(%sr0,%r5)
	fstw,m %fr6,%r4(%sr0,%r5)
	fstw,sm %fr6,%r4(%sr0,%r5)
	fstd %fr6,%r4(%sr0,%r5)
	fstd,s %fr6,%r4(%sr0,%r5)
	fstd,m %fr6,%r4(%sr0,%r5)
	fstd,sm %fr6,%r4(%sr0,%r5)
	fstqx %fr6,%r4(%sr0,%r5)
	fstqx,s %fr6,%r4(%sr0,%r5)
	fstqx,m %fr6,%r4(%sr0,%r5)
	fstqx,sm %fr6,%r4(%sr0,%r5)

	fldw 0(%sr0,%r5),%fr6
	fldw,mb 0(%sr0,%r5),%fr6
	fldw,ma 0(%sr0,%r5),%fr6
	fldd 0(%sr0,%r5),%fr6
	fldd,mb 0(%sr0,%r5),%fr6
	fldd,ma 0(%sr0,%r5),%fr6
	fstw %fr6,0(%sr0,%r5)
	fstw,mb %fr6,0(%sr0,%r5)
	fstw,ma %fr6,0(%sr0,%r5)
	fstd %fr6,0(%sr0,%r5)
	fstd,mb %fr6,0(%sr0,%r5)
	fstd,ma %fr6,0(%sr0,%r5)
	fstqs %fr6,0(%sr0,%r5)
	fstqs,mb %fr6,0(%sr0,%r5)
	fstqs,ma %fr6,0(%sr0,%r5)

	fldw 64(%r5),%fr6
	fldw,mb 64(%r5),%fr6
	fldw,ma 64(%r5),%fr6
	fldd 64(%r5),%fr6
	fldd,mb 64(%r5),%fr6
	fldd,ma 64(%r5),%fr6
	fstw %fr6,64(%r5)
	fstw,mb %fr6,64(%r5)
	fstw,ma %fr6,64(%r5)
	fstd %fr6,64(%r5)
	fstd,mb %fr6,64(%r5)
	fstd,ma %fr6,64(%r5)
