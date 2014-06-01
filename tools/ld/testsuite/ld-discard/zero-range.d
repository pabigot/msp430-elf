#source: zero-range.s
#ld: -T discard.ld
#objdump: -s -j .debug_ranges
#xfail: "cy16-*-*" "ip4k-*-*" "nios-*-*"

.*:     file format .*elf.*

Contents of section .debug_ranges:
 0000 (01)?000000(01)? (01)?000000(01)? 00000000 00000000 .*
