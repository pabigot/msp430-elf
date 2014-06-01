	.text
	.global labN
lab0:	.long	((lab0 - labN) + (. - 1))
	/* NB: local label below ("1:") is encoded with a ^B (\002) char in the symbol name! */
1:	.long	((lab0 * 1b) / (2 * .))
lab2:	.long	((lab2 & 0xdead) | .)
lab3:	.long	((- lab3) & (~ 1b) | (! lab2))
