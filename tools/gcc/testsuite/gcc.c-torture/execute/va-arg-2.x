if {[istarget "d30v-*-*"]} {

   # On the d30v there is a conflict between the function name 'f0'
   # and the register name 'f0'.  Specifying -C to the assembler
   # disables any warnings about this coflict

    set torture_eval_before_compile {
      set additional_flags "-Wa,-C"
    }
}

return 0
