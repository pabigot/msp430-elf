	.code
	.align 4
	.level 2.0
	.EXPORT integer_memory_tests,CODE
	.EXPORT integer_indexing_load,CODE
	.EXPORT integer_load_short_memory,CODE
	.EXPORT integer_store_short_memory,CODE
	.EXPORT main,CODE
	.EXPORT main,ENTRY,PRIV_LEV=3,RTNVAL=GR
; Basic integer memory tests which also test the various 
; addressing modes and completers.
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.
; 
integer_memory_tests: 
	ldw 0(%sr0,%r4),%r26
	ldh 0(%sr0,%r4),%r26
	ldb 0(%sr0,%r4),%r26
	stw %r26,0(%sr0,%r4)
	sth %r26,0(%sr0,%r4)
	stb %r26,0(%sr0,%r4)

; Should make sure pre/post modes are recognized correctly.
        ldw,ma 64(%sr0,%r4),%r26
        stw,ma %r26,64(%sr0,%r4)

integer_indexing_load: 
	ldw %r5(%sr0,%r4),%r26
	ldw,s %r5(%sr0,%r4),%r26
	ldw,m %r5(%sr0,%r4),%r26
	ldw,sm %r5(%sr0,%r4),%r26
	ldh %r5(%sr0,%r4),%r26
	ldh,s %r5(%sr0,%r4),%r26
	ldh,m %r5(%sr0,%r4),%r26
	ldh,sm %r5(%sr0,%r4),%r26
	ldb %r5(%sr0,%r4),%r26
	ldb,s %r5(%sr0,%r4),%r26
	ldb,m %r5(%sr0,%r4),%r26
	ldb,sm %r5(%sr0,%r4),%r26
	ldwa %r5(%r4),%r26
	ldwa,s %r5(%r4),%r26
	ldwa,m %r5(%r4),%r26
	ldwa,sm %r5(%r4),%r26
	ldcw %r5(%sr0,%r4),%r26
	ldcw,s %r5(%sr0,%r4),%r26
	ldcw,m %r5(%sr0,%r4),%r26
	ldcw,sm %r5(%sr0,%r4),%r26

integer_load_short_memory: 
	ldw 0(%sr0,%r4),%r26
	ldw,mb 0(%sr0,%r4),%r26
	ldw,ma 0(%sr0,%r4),%r26
	ldh 0(%sr0,%r4),%r26
	ldh,mb 0(%sr0,%r4),%r26
	ldh,ma 0(%sr0,%r4),%r26
	ldb 0(%sr0,%r4),%r26
	ldb,mb 0(%sr0,%r4),%r26
	ldb,ma 0(%sr0,%r4),%r26
	ldwa 0(%r4),%r26
	ldwa,mb 0(%r4),%r26
	ldwa,ma 0(%r4),%r26
	ldcw 0(%sr0,%r4),%r26
	ldcw,mb 0(%sr0,%r4),%r26
	ldcw,ma 0(%sr0,%r4),%r26

integer_store_short_memory: 
	stw %r26,0(%sr0,%r4)
	stw,mb %r26,0(%sr0,%r4)
	stw,ma %r26,0(%sr0,%r4)
	sth %r26,0(%sr0,%r4)
	sth,mb %r26,0(%sr0,%r4)
	sth,ma %r26,0(%sr0,%r4)
	stb %r26,0(%sr0,%r4)
	stb,mb %r26,0(%sr0,%r4)
	stb,ma %r26,0(%sr0,%r4)
	stwa %r26,0(%r4)
	stwa,mb %r26,0(%r4)
	stwa,ma %r26,0(%r4)
	stby %r26,0(%sr0,%r4)
	stby,b %r26,0(%sr0,%r4)
	stby,e %r26,0(%sr0,%r4)
	stby,b,m %r26,0(%sr0,%r4)
	stby,e,m %r26,0(%sr0,%r4)

        ldw,mb 64(%sr0,%r4),%r26
        stw,mb %r26,64(%sr0,%r4)
        ldw,ma -64(%sr0,%r4),%r26
        stw,ma %r26,-64(%sr0,%r4)

	ldcd %r5(%sr0,%r4),%r26
	ldcd,s %r5(%sr0,%r4),%r26
	ldcd,m %r5(%sr0,%r4),%r26
	ldcd,sm %r5(%sr0,%r4),%r26
	ldcd 8(%sr0,%r4),%r26
	ldcd,mb 8(%sr0,%r4),%r26
	ldcd,ma 8(%sr0,%r4),%r26

	ldd 64(%r4),%r26
	ldd %r5(%sr0,%r4),%r26
	ldd,s %r5(%sr0,%r4),%r26
	ldd,m %r5(%sr0,%r4),%r26
	ldd,sm %r5(%sr0,%r4),%r26
	ldd 8(%sr0,%r4),%r26
	ldd,mb 8(%sr0,%r4),%r26
	ldd,ma 8(%sr0,%r4),%r26

	ldwa %r5(%r4),%r26
	ldwa,s %r5(%r4),%r26
	ldwa,m %r5(%r4),%r26
	ldwa,sm %r5(%r4),%r26
	ldwa 8(%r4),%r26
	ldwa,mb 8(%r4),%r26
	ldwa,ma 8(%r4),%r26

	std %r26,64(%r4)
	std %r26,8(%sr0,%r4)
	std,mb %r26,8(%sr0,%r4)
	std,ma %r26,8(%sr0,%r4)

	stda %r26,0(%r4)
	stda,mb %r26,0(%r4)
	stda,ma %r26,0(%r4)
	stda %r26,8(%r4)
	stda,mb %r26,8(%r4)
	stda,ma %r26,8(%r4)

	stdby %r26,0(%sr0,%r4)
	stdby,b %r26,0(%sr0,%r4)
	stdby,e %r26,0(%sr0,%r4)
	stdby,b,m %r26,0(%sr0,%r4)
	stdby,e,m %r26,0(%sr0,%r4)

# Regression tests
	STD,MA  %r3,144(%r30)
	LDD,MA  -144(%r30),%r3
