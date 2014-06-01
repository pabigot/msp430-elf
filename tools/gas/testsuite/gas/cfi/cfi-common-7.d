#objdump: -Wf
#name: CFI common 7
#...
Contents of the .eh_frame section:

0+000000 0+000010 0+000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01]b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+000014 0+0000(18|1c|20) 0+000018 FDE cie=0+000000 pc=.*
  DW_CFA_advance_loc: 16 to .*
  DW_CFA_def_cfa: r0( \([er]ax\)|) ofs 16
  DW_CFA_advance_loc[24]: 75040 to .*
  DW_CFA_def_cfa: r0( \([er]ax\)|) ofs 64
#...
