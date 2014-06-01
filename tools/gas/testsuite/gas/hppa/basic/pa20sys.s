	.code

	.level 2.0
	.align 4
	idtlbt %r15,%r16
	iitlbt %r1,%r2
        pdtlb %r4(%sr0,%r5)
        pdtlb,m %r4(%sr0,%r5)
        pdtlb,l %r4(%sr0,%r5)
        pdtlb,l,m %r4(%sr0,%r5)
        pitlb %r4(%sr0,%r5)
        pitlb,m %r4(%sr0,%r5)
        pitlb,l %r4(%sr0,%r5)
        pitlb,l,m %r4(%sr0,%r5)
        probe,r (%sr0,%r5),%r6,%r7
        probe,w (%sr0,%r5),%r6,%r7
        probei,r (%sr0,%r5),1,%r7
        probei,w (%sr0,%r5),1,%r7
	rfi
	rfi,r


