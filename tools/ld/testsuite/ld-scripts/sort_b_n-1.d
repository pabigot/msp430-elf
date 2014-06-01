#source: sort_b_n-1.s
#ld: -T sort.t --sort-section name
#ld_after_inputfiles: -X
#name: --sort-section name
#nm: -n

#...
0[0-9a-f]* t text
#...
0[0-9a-f]* t text1
#...
0[0-9a-f]* t text2
#...
0[0-9a-f]* t text3
#pass
