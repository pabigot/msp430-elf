#objdump: -Wf
#name: CFI on x86-64
#...
Contents of the .eh_frame section:

0+000000 0+000014 0+000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: (16|32)
  Augmentation data:     1b

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_offset: (r16 \(rip\)|r32 \(xmm15\)) at cfa-8
  DW_CFA_nop
  DW_CFA_nop

0+000018 0+000014 0+00001c FDE cie=0+000000 pc=0+000000..0+000014
  DW_CFA_advance_loc: 7 to 0+000007
  DW_CFA_def_cfa_offset: 4668
  DW_CFA_advance_loc: 12 to 0+000013
  DW_CFA_def_cfa_offset: 8

0+000030 0+00001c 0+000034 FDE cie=0+000000 pc=0+000014..0+000022
  DW_CFA_advance_loc: 1 to 0+000015
  DW_CFA_def_cfa_offset: 16
  DW_CFA_offset: r6 \(rbp\) at cfa-16
  DW_CFA_advance_loc: 3 to 0+000018
  DW_CFA_def_cfa_register: r6 \(rbp\)
  DW_CFA_advance_loc: 9 to 0+000021
  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+000050 0+000014 0+000054 FDE cie=0+000000 pc=0+000022..0+000035
  DW_CFA_advance_loc: 3 to 0+000025
  DW_CFA_def_cfa_register: r8 \(r8\)
  DW_CFA_advance_loc: 15 to 0+000034
  DW_CFA_def_cfa_register: r7 \(rsp\)
  DW_CFA_nop

0+000068 0+000010 0+00006c FDE cie=0+000000 pc=0+000035..0+00003b
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00007c 0+000010 0+000080 FDE cie=0+000000 pc=0+00003b..0+00004d
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+000090 0+000010 0+000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: (16|32)
  Augmentation data:     1b

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8

0+0000a4 0+00002c 0+000018 FDE cie=0+000090 pc=0+00004d..0+000058
  DW_CFA_advance_loc: 1 to 0+00004e
  DW_CFA_def_cfa_offset: 16
  DW_CFA_advance_loc: 1 to 0+00004f
  DW_CFA_def_cfa_register: r8 \(r8\)
  DW_CFA_advance_loc: 1 to 0+000050
  DW_CFA_def_cfa_offset: 4676
  DW_CFA_advance_loc: 1 to 0+000051
  DW_CFA_offset_extended_sf: r4 \(rsi\) at cfa\+16
  DW_CFA_advance_loc: 1 to 0+000052
  DW_CFA_register: r8 \(r8\) in r9 \(r9\)
  DW_CFA_advance_loc: 1 to 0+000053
  DW_CFA_remember_state
  DW_CFA_advance_loc: 1 to 0+000054
  DW_CFA_restore: r6 \(rbp\)
  DW_CFA_advance_loc: 1 to 0+000055
  DW_CFA_undefined: r16 \(rip\)
  DW_CFA_advance_loc: 1 to 0+000056
  DW_CFA_same_value: r3 \(rbx\)
  DW_CFA_advance_loc: 1 to 0+000057
  DW_CFA_restore_state
  DW_CFA_nop

0+0000d4 0+000010 0+000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: (16|32)
  Augmentation data:     1b

  DW_CFA_undefined: r16 \(rip\)
  DW_CFA_nop

0+0000e8 0+0000c[8c] 0+000018 FDE cie=0+0000d4 pc=0+000058..0+000097
  DW_CFA_advance_loc: 1 to 0+000059
  DW_CFA_undefined: r0 \(rax\)
  DW_CFA_advance_loc: 1 to 0+00005a
  DW_CFA_undefined: r2 \(rcx\)
  DW_CFA_advance_loc: 1 to 0+00005b
  DW_CFA_undefined: r1 \(rdx\)
  DW_CFA_advance_loc: 1 to 0+00005c
  DW_CFA_undefined: r3 \(rbx\)
  DW_CFA_advance_loc: 1 to 0+00005d
  DW_CFA_undefined: r7 \(rsp\)
  DW_CFA_advance_loc: 1 to 0+00005e
  DW_CFA_undefined: r6 \(rbp\)
  DW_CFA_advance_loc: 1 to 0+00005f
  DW_CFA_undefined: r4 \(rsi\)
  DW_CFA_advance_loc: 1 to 0+000060
  DW_CFA_undefined: r5 \(rdi\)
  DW_CFA_advance_loc: 1 to 0+000061
  DW_CFA_undefined: r8 \(r8\)
  DW_CFA_advance_loc: 1 to 0+000062
  DW_CFA_undefined: r9 \(r9\)
  DW_CFA_advance_loc: 1 to 0+000063
  DW_CFA_undefined: r10 \(r10\)
  DW_CFA_advance_loc: 1 to 0+000064
  DW_CFA_undefined: r11 \(r11\)
  DW_CFA_advance_loc: 1 to 0+000065
  DW_CFA_undefined: r12 \(r12\)
  DW_CFA_advance_loc: 1 to 0+000066
  DW_CFA_undefined: r13 \(r13\)
  DW_CFA_advance_loc: 1 to 0+000067
  DW_CFA_undefined: r14 \(r14\)
  DW_CFA_advance_loc: 1 to 0+000068
  DW_CFA_undefined: r15 \(r15\)
  DW_CFA_advance_loc: 1 to 0+000069
  DW_CFA_undefined: r49 \([er]flags\)
  DW_CFA_advance_loc: 1 to 0+00006a
  DW_CFA_undefined: r50 \(es\)
  DW_CFA_advance_loc: 1 to 0+00006b
  DW_CFA_undefined: r51 \(cs\)
  DW_CFA_advance_loc: 1 to 0+00006c
  DW_CFA_undefined: r53 \(ds\)
  DW_CFA_advance_loc: 1 to 0+00006d
  DW_CFA_undefined: r52 \(ss\)
  DW_CFA_advance_loc: 1 to 0+00006e
  DW_CFA_undefined: r54 \(fs\)
  DW_CFA_advance_loc: 1 to 0+00006f
  DW_CFA_undefined: r55 \(gs\)
  DW_CFA_advance_loc: 1 to 0+000070
  DW_CFA_undefined: r62 \(tr\)
  DW_CFA_advance_loc: 1 to 0+000071
  DW_CFA_undefined: r63 \(ldtr\)
  DW_CFA_advance_loc: 1 to 0+000072
  DW_CFA_undefined: r58 \(fs\.base\)
  DW_CFA_advance_loc: 1 to 0+000073
  DW_CFA_undefined: r59 \(gs\.base\)
  DW_CFA_advance_loc: 1 to 0+000074
  DW_CFA_undefined: r64 \(mxcsr\)
  DW_CFA_advance_loc: 1 to 0+000075
  DW_CFA_undefined: r17 \(xmm0\)
  DW_CFA_advance_loc: 1 to 0+000076
  DW_CFA_undefined: r18 \(xmm1\)
  DW_CFA_advance_loc: 1 to 0+000077
  DW_CFA_undefined: r19 \(xmm2\)
  DW_CFA_advance_loc: 1 to 0+000078
  DW_CFA_undefined: r20 \(xmm3\)
  DW_CFA_advance_loc: 1 to 0+000079
  DW_CFA_undefined: r21 \(xmm4\)
  DW_CFA_advance_loc: 1 to 0+00007a
  DW_CFA_undefined: r22 \(xmm5\)
  DW_CFA_advance_loc: 1 to 0+00007b
  DW_CFA_undefined: r23 \(xmm6\)
  DW_CFA_advance_loc: 1 to 0+00007c
  DW_CFA_undefined: r24 \(xmm7\)
  DW_CFA_advance_loc: 1 to 0+00007d
  DW_CFA_undefined: r25 \(xmm8\)
  DW_CFA_advance_loc: 1 to 0+00007e
  DW_CFA_undefined: r26 \(xmm9\)
  DW_CFA_advance_loc: 1 to 0+00007f
  DW_CFA_undefined: r27 \(xmm10\)
  DW_CFA_advance_loc: 1 to 0+000080
  DW_CFA_undefined: r28 \(xmm11\)
  DW_CFA_advance_loc: 1 to 0+000081
  DW_CFA_undefined: r29 \(xmm12\)
  DW_CFA_advance_loc: 1 to 0+000082
  DW_CFA_undefined: r30 \(xmm13\)
  DW_CFA_advance_loc: 1 to 0+000083
  DW_CFA_undefined: r31 \(xmm14\)
  DW_CFA_advance_loc: 1 to 0+000084
  DW_CFA_undefined: r32 \(xmm15\)
  DW_CFA_advance_loc: 1 to 0+000085
  DW_CFA_undefined: r65 \(fcw\)
  DW_CFA_advance_loc: 1 to 0+000086
  DW_CFA_undefined: r66 \(fsw\)
  DW_CFA_advance_loc: 1 to 0+000087
  DW_CFA_undefined: r33 \(st\(?0?\)?\)
  DW_CFA_advance_loc: 1 to 0+000088
  DW_CFA_undefined: r34 \(st\(?1\)?\)
  DW_CFA_advance_loc: 1 to 0+000089
  DW_CFA_undefined: r35 \(st\(?2\)?\)
  DW_CFA_advance_loc: 1 to 0+00008a
  DW_CFA_undefined: r36 \(st\(?3\)?\)
  DW_CFA_advance_loc: 1 to 0+00008b
  DW_CFA_undefined: r37 \(st\(?4\)?\)
  DW_CFA_advance_loc: 1 to 0+00008c
  DW_CFA_undefined: r38 \(st\(?5\)?\)
  DW_CFA_advance_loc: 1 to 0+00008d
  DW_CFA_undefined: r39 \(st\(?6\)?\)
  DW_CFA_advance_loc: 1 to 0+00008e
  DW_CFA_undefined: r40 \(st\(?7\)?\)
  DW_CFA_advance_loc: 1 to 0+00008f
  DW_CFA_undefined: r41 \(mm0\)
  DW_CFA_advance_loc: 1 to 0+000090
  DW_CFA_undefined: r42 \(mm1\)
  DW_CFA_advance_loc: 1 to 0+000091
  DW_CFA_undefined: r43 \(mm2\)
  DW_CFA_advance_loc: 1 to 0+000092
  DW_CFA_undefined: r44 \(mm3\)
  DW_CFA_advance_loc: 1 to 0+000093
  DW_CFA_undefined: r45 \(mm4\)
  DW_CFA_advance_loc: 1 to 0+000094
  DW_CFA_undefined: r46 \(mm5\)
  DW_CFA_advance_loc: 1 to 0+000095
  DW_CFA_undefined: r47 \(mm6\)
  DW_CFA_advance_loc: 1 to 0+000096
  DW_CFA_undefined: r48 \(mm7\)
  DW_CFA_nop
#pass
