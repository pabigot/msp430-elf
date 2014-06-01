	.code
	.level 2.0
; Needs to check lots of stuff (like corner bit cases)
bb_tests
	bb,< %r4,%sar,bb_tests
	bb,>= %r4,%sar,bb_tests
	bb,<,n %r4,%sar,bb_tests
	bb,>=,n %r4,%sar,bb_tests
	bb,*< %r4,5,bb_tests
	bb,*>= %r4,5,bb_tests
	bb,*<,n %r4,5,bb_tests
	bb,*>=,n %r4,5,bb_tests
	bb,*< %r4,35,bb_tests
	bb,*>= %r4,35,bb_tests
	bb,*<,n %r4,35,bb_tests
	bb,*>=,n %r4,35,bb_tests




