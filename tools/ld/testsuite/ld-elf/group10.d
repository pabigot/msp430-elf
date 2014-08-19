#source: group10.s
#ld: -r -T group.ld
#readelf: -Sg --wide
#notarget: msp430-*-*
# MSP430 has its own orphan placing code.

#...
group section \[[ 0-9]+\] `\.group' \[foo_group\] contains 4 sections:
   \[Index\]    Name
   \[[ 0-9]+\]   \.text.*
   \[[ 0-9]+\]   \.rodata\.str.*
   \[[ 0-9]+\]   \.data.*
   \[[ 0-9]+\]   \.keepme.*
