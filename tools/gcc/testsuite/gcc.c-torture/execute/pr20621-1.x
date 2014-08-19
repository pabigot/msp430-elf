# (In 64-bit mode) the array is so big that the remaining local
# data (in libpmon) gets pushed out of the small data area.

if { [check_effective_target_mips_newabi_large_long_double] } {
        return 1; 
}

return 0;
