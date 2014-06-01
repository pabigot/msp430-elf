# This doesn't work on for arm-coff in big-endian mode

set torture_eval_before_execute {

    global compiler_conditional_xfail_data

    set compiler_conditional_xfail_data {
	    "Loop optimiser bug" \
	    "xscale-*-coff *arm-*-coff" \
	    { "-mbig-endian" } \
	    { "" }
	}
    }

return 0
