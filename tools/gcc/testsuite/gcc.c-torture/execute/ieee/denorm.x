# The RX does not implement comparisons of denormalized and normalized
# numbers properly (even with the DN bit set).  So expext this test to fail.

set torture_eval_before_execute {

    set compiler_conditional_xfail_data {
        "RX fp insns do not handle denorms" \
        "rx-*-*" \
        { "*" } \
        { "-m64bit-doubles" }
        }    
}

return 0
