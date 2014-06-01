	.code
	.align 4
	.level 2.0
; More branching instructions than you ever knew what to do with.
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.
label1:
	cmpb %r0,%r4,label1
	cmpb,= %r0,%r4,label1
	cmpb,< %r0,%r4,label1
	cmpb,<= %r0,%r4,label1
	cmpb,<< %r0,%r4,label1
	cmpb,<<= %r0,%r4,label1
	cmpb,sv %r0,%r4,label1
	cmpb,od %r0,%r4,label1
	cmpb,tr %r0,%r4,label1
	cmpb,<> %r0,%r4,label1
	cmpb,>= %r0,%r4,label1
	cmpb,> %r0,%r4,label1
	cmpb,>>= %r0,%r4,label1
	cmpb,>> %r0,%r4,label1
	cmpb,nsv %r0,%r4,label1
	cmpb,ev %r0,%r4,label1
label2:
	cmpb,n %r0,%r4,label2
	cmpb,=,n %r0,%r4,label2
	cmpb,<,n %r0,%r4,label2
	cmpb,<=,n %r0,%r4,label2
	cmpb,<<,n %r0,%r4,label2
	cmpb,<<=,n %r0,%r4,label2
	cmpb,sv,n %r0,%r4,label2
	cmpb,od,n %r0,%r4,label2
	cmpb,tr,n %r0,%r4,label2
	cmpb,<>,n %r0,%r4,label2
	cmpb,>=,n %r0,%r4,label2
	cmpb,>,n %r0,%r4,label2
	cmpb,>>=,n %r0,%r4,label2
	cmpb,>>,n %r0,%r4,label2
	cmpb,nsv,n %r0,%r4,label2
	cmpb,ev,n %r0,%r4,label2
label3:
	cmpb,* %r0,%r4,label3
	cmpb,*= %r0,%r4,label3
	cmpb,*< %r0,%r4,label3
	cmpb,*<= %r0,%r4,label3
	cmpb,*<< %r0,%r4,label3
	cmpb,*<<= %r0,%r4,label3
	cmpb,*sv %r0,%r4,label3
	cmpb,*od %r0,%r4,label3
	cmpb,*tr %r0,%r4,label3
	cmpb,*<> %r0,%r4,label3
	cmpb,*>= %r0,%r4,label3
	cmpb,*> %r0,%r4,label3
	cmpb,*>>= %r0,%r4,label3
	cmpb,*>> %r0,%r4,label3
	cmpb,*nsv %r0,%r4,label3
	cmpb,*ev %r0,%r4,label3
label4:
	cmpb,*,n %r0,%r4,label4
	cmpb,*=,n %r0,%r4,label4
	cmpb,*<,n %r0,%r4,label4
	cmpb,*<=,n %r0,%r4,label4
	cmpb,*<<,n %r0,%r4,label4
	cmpb,*<<=,n %r0,%r4,label4
	cmpb,*sv,n %r0,%r4,label4
	cmpb,*od,n %r0,%r4,label4
	cmpb,*tr,n %r0,%r4,label4
	cmpb,*<>,n %r0,%r4,label4
	cmpb,*>=,n %r0,%r4,label4
	cmpb,*>,n %r0,%r4,label4
	cmpb,*>>=,n %r0,%r4,label4
	cmpb,*>>,n %r0,%r4,label4
	cmpb,*nsv,n %r0,%r4,label4
	cmpb,*ev,n %r0,%r4,label4
