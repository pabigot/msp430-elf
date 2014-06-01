	.code
        .level 2.0w
addb_tests
        addb,*= %r1,%r4,addb_tests
        addb,*< %r1,%r4,addb_tests
        addb,*<= %r1,%r4,addb_tests
        addb,*<> %r1,%r4,addb_tests
        addb,*>= %r1,%r4,addb_tests
        addb,*> %r1,%r4,addb_tests
        addb,*=,n %r1,%r4,addb_tests
        addb,*<,n %r1,%r4,addb_tests
        addb,*<=,n %r1,%r4,addb_tests
        addb,*<>,n %r1,%r4,addb_tests
        addb,*>=,n %r1,%r4,addb_tests
        addb,*>,n %r1,%r4,addb_tests
        addib,*= 1,%r4,addb_tests
        addib,*< 1,%r4,addb_tests
        addib,*<= 1,%r4,addb_tests
        addib,*<> 1,%r4,addb_tests
        addib,*>= 1,%r4,addb_tests
        addib,*> 1,%r4,addb_tests
        addib,*=,n 1,%r4,addb_tests
        addib,*<,n 1,%r4,addb_tests
        addib,*<=,n 1,%r4,addb_tests
        addib,*<>,n 1,%r4,addb_tests
        addib,*>=,n 1,%r4,addb_tests
        addib,*>,n 1,%r4,addb_tests



