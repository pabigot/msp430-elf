#objdump: -Wf
#name: CFI common 2
#...
Contents of the .eh_frame section:

0+000000 0+000010 0+000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01]b
#...
0+000014 0+0000[12][c0] 0+000018 FDE cie=0+000000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0( \([er]ax\)|) ofs 16
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_remember_state
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa_offset: 0
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_restore_state
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa_offset: 0
#pass
