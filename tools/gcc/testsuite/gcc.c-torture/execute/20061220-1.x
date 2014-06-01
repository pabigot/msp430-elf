# asm statements for the RL78 have to be very carefully constructed. 
if { [istarget "rl78-*-*"] } {
        return 1;
}

return 0
