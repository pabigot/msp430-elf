	.code
	.align 4
	.level 2.0
	fmpyfadd,sgl %fr4R,%fr6R,%fr8R,%fr10R
	fmpyfadd,dbl %fr4,%fr6,%fr8,%fr10
	fmpynfadd,sgl %fr4R,%fr6R,%fr8R,%fr10R
	fmpynfadd,dbl %fr4,%fr6,%fr8,%fr10
	fneg,sgl %fr4R,%fr6R
	fneg,dbl %fr4,%fr6
	fnegabs,sgl %fr4R,%fr6R
	fnegabs,dbl %fr4,%fr6
	# No need to test all the completers, we're really concerned
	# about handling of the CA argument.
	fcmp,dbl,= %fr4,%fr5,0
	fcmp,dbl,= %fr4,%fr5,6
	fid
	ftest 0
	ftest 6
	ftest,acc
	ftest,acc2
	ftest,acc4
	ftest,acc6
	ftest,acc8
	ftest,rej
	ftest,rej8
	fcnv,sgl,dbl %fr5,%fr6
	fcnv,sgl,w %fr5,%fr6
	fcnv,sgl,uw %fr5,%fr6
	fcnv,sgl,dw %fr5,%fr6
	fcnv,sgl,udw %fr5,%fr6
	fcnv,dbl,w %fr5,%fr6
	fcnv,dbl,uw %fr5,%fr6
	fcnv,dbl,dw %fr5,%fr6
	fcnv,dbl,udw %fr5,%fr6
	fcnv,t,sgl,w %fr5,%fr6
	fcnv,t,sgl,uw %fr5,%fr6
	fcnv,t,sgl,dw %fr5,%fr6
	fcnv,t,sgl,udw %fr5,%fr6
	fcnv,t,dbl,w %fr5,%fr6
	fcnv,t,dbl,uw %fr5,%fr6
	fcnv,t,dbl,dw %fr5,%fr6
	fcnv,t,dbl,udw %fr5,%fr6
	fcnv,w,sgl %fr5,%fr6
	fcnv,uw,sgl %fr5,%fr6
	fcnv,dw,sgl %fr5,%fr6
	fcnv,udw,sgl %fr5,%fr6
	fcnv,w,dbl %fr5,%fr6
	fcnv,uw,dbl %fr5,%fr6
	fcnv,dw,dbl %fr5,%fr6
	fcnv,udw,dbl %fr5,%fr6
	.END
