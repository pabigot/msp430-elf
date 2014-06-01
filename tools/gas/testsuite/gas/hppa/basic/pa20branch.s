	.code
	.align 4
	.level 2.0
; More branching instructions than you ever knew what to do with.
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.
label1:
	be 0x1234(%sr1,%r2)
	be,n 0x1234(%sr1,%r2)
	be,l 0x1234(%sr1,%r2),%sr0,%r31
	be,l,n 0x1234(%sr1,%r2),%sr0,%r31
	bve (%r3)
	bve,n (%r3)
	bve,pop (%r3)
	bve,pop,n (%r3)
	bve (%r3)
	bve,n (%r3)
	bve,l (%r3),%r2
	bve,l,push (%r3),%r2
	bve,l,n (%r3),%r2
	bve,l,push,n (%r3),%r2
