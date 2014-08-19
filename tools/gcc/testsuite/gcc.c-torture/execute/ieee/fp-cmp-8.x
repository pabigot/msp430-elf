if [istarget "rx-*-*"] {
  set torture_eval_before_execute {
    global compiler_conditional_xfail_data
    set compiler_conditional_xfail_data {
        "RX fp isnsn do not handle denorms" \
        "rx-*-*" \
        { "*" } \
        { "-m64bit-doubles" }
    }
  }
}

return 0
