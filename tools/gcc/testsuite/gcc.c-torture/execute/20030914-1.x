# (In 64-bit mode) TI-mode math uses for long doubles
# but libgcc does not support them.

load_lib target-supports.exp

if { [check_effective_target_mips_newabi_large_long_double] } {
        return 1; 
}

return 0;