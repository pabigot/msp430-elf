# This doesn't work on d10v if doubles are not 64 bits

if { [istarget "d10v-*-*"] && ! [string-match "*-mdouble64*" $CFLAGS] } {
	set torture_execute_xfail "d10v-*-*"
}
if [istarget "avr-*-*"] {
    # AVR doubles are floats
    return 1
}
set torture_eval_before_execute {

    set compiler_conditional_xfail_data {
        "RX uses 32-bit doubles by default" \
        "rx-*-*" \
        { "-O0" "-O1" } \
        { "-m64bit-doubles" }
        }    
}

return 0
