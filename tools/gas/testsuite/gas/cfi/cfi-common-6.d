#objdump: -Wf
#name: CFI common 6
#...
Contents of the .eh_frame section:

0+000000 0+000018 0+000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     03 .. .. .. .. 0c 1b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00001c 0+000018 0+000020 FDE cie=0+000000 pc=0+0000(00|24)..0+0000(04|28)
  Augmentation data:     (00 00 00 00 de ad be ef|ef be ad de 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+000038 0+000010 0+000000 CIE
  Version:               1
  Augmentation:          "zLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     0c 1b

  DW_CFA_nop

0+00004c 0+000018 0+000018 FDE cie=0+000038 pc=0+0000(04|58)..0+0000(08|5c)
  Augmentation data:     (00 00 00 00 de ad be ef|ef be ad de 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+000068 0+000018 0+00006c FDE cie=0+000000 pc=0+0000(08|78)..0+0000(0c|7c)
  Augmentation data:     (00 00 00 00 be ef de ad|ad de ef be 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+000084 0+000018 0+000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     1b .. .. .. .. 1b 1b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0000a0 0+000014 0+000020 FDE cie=0+000084 pc=0+0000(0c|b4)..0+0000(10|b8)
  Augmentation data:     .. .. .. ..

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0000b8 0+000014 0+000038 FDE cie=0+000084 pc=0+0000(10|d0)..0+0000(14|d4)
  Augmentation data:     .. .. .. ..

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

