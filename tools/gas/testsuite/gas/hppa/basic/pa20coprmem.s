	.code
	.align 4
	.level 2.0
; Basic copr memory tests which also test the various 
; addressing modes and completers.
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.
; 
copr_indexing_load 

	cldw,4 %r5(%sr0,%r4),%r26
	cldw,4,s %r5(%sr0,%r4),%r26
	cldw,4,m %r5(%sr0,%r4),%r26
	cldw,4,sm %r5(%sr0,%r4),%r26
	cldd,4 %r5(%sr0,%r4),%r26
	cldd,4,s %r5(%sr0,%r4),%r26
	cldd,4,m %r5(%sr0,%r4),%r26
	cldd,4,sm %r5(%sr0,%r4),%r26

copr_indexing_store 
	cstw,4 %r26,%r5(%sr0,%r4)
	cstw,4,s %r26,%r5(%sr0,%r4)
	cstw,4,m %r26,%r5(%sr0,%r4)
	cstw,4,sm %r26,%r5(%sr0,%r4)
	cstd,4 %r26,%r5(%sr0,%r4)
	cstd,4,s %r26,%r5(%sr0,%r4)
	cstd,4,m %r26,%r5(%sr0,%r4)
	cstd,4,sm %r26,%r5(%sr0,%r4)

copr_short_memory 
	cldw,4 0(%sr0,%r4),%r26
	cldw,4,mb 0(%sr0,%r4),%r26
	cldw,4,ma 0(%sr0,%r4),%r26
	cldd,4 0(%sr0,%r4),%r26
	cldd,4,mb 0(%sr0,%r4),%r26
	cldd,4,ma 0(%sr0,%r4),%r26
	cstw,4 %r26,0(%sr0,%r4)
	cstw,4,mb %r26,0(%sr0,%r4)
	cstw,4,ma %r26,0(%sr0,%r4)
	cstd,4 %r26,0(%sr0,%r4)
	cstd,4,mb %r26,0(%sr0,%r4)
	cstd,4,ma %r26,0(%sr0,%r4)

; gas fucks this up thinks it gets the expression 4 modulo 5
;	cldwx,4 %r5(0,%r4),%r%r26
