if { [istarget "nios2-*-*"] } {
    # This test can cause the stack to underflow on Nios II.
    set torture_execute_xfail [istarget]
}

if [istarget "rx-*-*"] {
	# Builtin apply not yet implemented
	return 1
}

return 0
