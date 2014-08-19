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

# (In 64-bit mode) TI-mode math is used for long doubles
# but libgcc does not support this.

set torture_eval_before_compile {
  if {[string match {*-mabi=64*} "$option"]} {
    return 1;
  }
}

return 0
