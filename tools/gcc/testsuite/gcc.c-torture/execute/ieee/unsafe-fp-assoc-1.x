if [istarget "avr-*-*"] {
    # AVR doubles are floats
    return 1
}
if { [istarget "rx-*-*"] } {
	set torture_execute_xfail "rx-*-*"
}
return 0
