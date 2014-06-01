# The h8300 has only single precision, hence this test fails.
if { [istarget "h8*-*-*"] } {
        set torture_execute_xfail "h8*-*-*"
}
return 0
