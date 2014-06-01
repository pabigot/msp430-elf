#objdump: -dr --prefix-addresses --show-raw-insn
#name: V850E3V5 instruction tests
#as: -mv850e3v5

# Test the new instructions in the V850E3V5 processor

.*: +file format .*v850.*

Disassembly of section .text:
0x0+00 f8 07 78 cb[ 	]*ldl.w[ 	]*\[r24\], r25
0x0+04 fb 07 7a d3[ 	]*stc.w[ 	]*r26, \[r27\]
0x0+08 fc ef d4 40[ 	]*bins[ 	]*r28, 2, 3, r29
0x0+0c e4 2f c4 30[ 	]*rotl[ 	]*4, r5, r6
0x0+10 e7 47 c6 48[ 	]*rotl[ 	]*r7, r8, r9
0x0+14 aa 07 69 65 68 e4[ 	]*ld.dw[ 	]*-904106\[r10\], r12
0x0+1a ad 07 6f 65 68 24[ 	]*st.dw[ 	]*r12, 1193046\[r13\]
0x0+20 e2 47 42 4c[ 	]*cvtf.hs[ 	]*r8, r9
0x0+24 e3 57 42 5c[ 	]*cvtf.sh[ 	]*r10, r11
0x0+28 ec 6f e0 74[ 	]*fmaf.s[ 	]*r12, r13, r14
0x0+2c ef 87 e2 8c[ 	]*fmsf.s[ 	]*r15, r16, r17
0x0+30 f2 9f e4 a4[ 	]*fnmaf.s[ 	]*r18, r19, r20
0x0+34 f5 b7 e6 bc[ 	]*fnmsf.s[ 	]*r21, r22, r23
0x0+38 ee df 60 01[ 	]*pref[ 	]*prefi, \[r14\]
0x0+3c ef df 60 21[ 	]*pref[ 	]*prefd, \[r15\]
0x0+40 f0 f7 60 21[ 	]*cache[ 	]*cfald, \[r16\]
0x0+44 f1 f7 60 01[ 	]*cache[ 	]*cfali, \[r17\]
0x0+48 f2 e7 60 21[ 	]*cache[ 	]*chbid, \[r18\]
0x0+4c f3 e7 60 01[ 	]*cache[ 	]*chbii, \[r19\]
0x0+50 f4 e7 60 31[ 	]*cache[ 	]*chbiwbd, \[r20\]
0x0+54 f5 e7 60 39[ 	]*cache[ 	]*chbwbd, \[r21\]
0x0+58 f6 ef 60 21[ 	]*cache[ 	]*cibid, \[r22\]
0x0+5c f7 ef 60 01[ 	]*cache[ 	]*cibii, \[r23\]
0x0+60 f8 ef 60 31[ 	]*cache[ 	]*cibiwbd, \[r24\]
0x0+64 f9 ef 60 39[ 	]*cache[ 	]*cibwbd, \[r25\]
0x0+68 fa ff 60 29[ 	]*cache[ 	]*cildd, \[r26\]
0x0+6c fb ff 60 09[ 	]*cache[ 	]*cildi, \[r27\]
0x0+70 fc ff 60 21[ 	]*cache[ 	]*cistd, \[r28\]
0x0+74 fd ff 60 01[ 	]*cache[ 	]*cisti, \[r29\]
