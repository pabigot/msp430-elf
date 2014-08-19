if { [istarget "alpha*-*-*"] || [istarget "sh*-*-*"] } {
	# alpha and SH require -mieee for this test.
	set additional_flags "-mieee"
}
if [istarget "spu-*-*"] {
	# No Inf/NaN support on SPU.
	return 1
}

if [istarget "rx-*-*"] {
	# No Denormal support on RX.
	return 1
}

return 0
