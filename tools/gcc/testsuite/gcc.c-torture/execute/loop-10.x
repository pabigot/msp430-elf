set torture_eval_before_execute {
	global compiler_conditional_xfail_data

	set compiler_conditional_xfail_data {
		"Loop optimiser bug" \
		"xscale-*-* *arm-*-*" \
		"-O2 -Os"
	}
}

return 0
