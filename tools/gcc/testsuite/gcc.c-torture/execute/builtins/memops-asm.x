if [istarget "msp430-*-*"] {
    # Linker relaxation means that the size of the
    # memmove function changes when using LTO.
    set torture_eval_before_compile {
      if {[string match {*-flto*} "$option"]} {
        continue
      }
    }
}

return 0
