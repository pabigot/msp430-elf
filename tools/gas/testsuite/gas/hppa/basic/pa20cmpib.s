	.code
	.align 4
	.level 2.0
; More branching instructions than you ever knew what to do with.
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.
label1:
	cmpib 5,%r4,label1
	cmpib,= 5,%r4,label1
	cmpib,< 5,%r4,label1
	cmpib,<= 5,%r4,label1
	cmpib,<< 5,%r4,label1
	cmpib,<<= 5,%r4,label1
	cmpib,sv 5,%r4,label1
	cmpib,od 5,%r4,label1
	cmpib,tr 5,%r4,label1
	cmpib,<> 5,%r4,label1
	cmpib,>= 5,%r4,label1
	cmpib,> 5,%r4,label1
	cmpib,>>= 5,%r4,label1
	cmpib,>> 5,%r4,label1
	cmpib,nsv 5,%r4,label1
	cmpib,ev 5,%r4,label1
label2:
	cmpib,n 5,%r4,label2
	cmpib,=,n 5,%r4,label2
	cmpib,<,n 5,%r4,label2
	cmpib,<=,n 5,%r4,label2
	cmpib,<<,n 5,%r4,label2
	cmpib,<<=,n 5,%r4,label2
	cmpib,sv,n 5,%r4,label2
	cmpib,od,n 5,%r4,label2
	cmpib,tr,n 5,%r4,label2
	cmpib,<>,n 5,%r4,label2
	cmpib,>=,n 5,%r4,label2
	cmpib,>,n 5,%r4,label2
	cmpib,>>=,n 5,%r4,label2
	cmpib,>>,n 5,%r4,label2
	cmpib,nsv,n 5,%r4,label2
	cmpib,ev,n 5,%r4,label2
label3:
	cmpib,*= 5,%r4,label3
	cmpib,*< 5,%r4,label3
	cmpib,*<= 5,%r4,label3
	cmpib,*<< 5,%r4,label3
	cmpib,*<> 5,%r4,label3
	cmpib,*>= 5,%r4,label3
	cmpib,*> 5,%r4,label3
	cmpib,*>>= 5,%r4,label3
	cmpib,*=,n 5,%r4,label3
	cmpib,*<,n 5,%r4,label3
	cmpib,*<=,n 5,%r4,label3
	cmpib,*<<,n 5,%r4,label3
	cmpib,*<>,n 5,%r4,label3
	cmpib,*>=,n 5,%r4,label3
	cmpib,*>,n 5,%r4,label3
	cmpib,*>>=,n 5,%r4,label3
