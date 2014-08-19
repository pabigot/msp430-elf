/* { dg-do run } */
/* { dg-options "-mrelax" } */
/* { dg-output ".*6.*" } */
        .file   "issue112045.S"
        .section        .rodata
.LC0:
        .string "%d\n"

        .section .text
        .global _main
        .type   _main, @function
_main:
        movm [a3], (sp)
        mov  sp, a3
        add  -12, sp
        call _sub, [], 0
        mov  d0, d1
        mov  .LC0, d0
        call _printf, [], 0
        mov  a3, sp
	mov  0, d0
        ret  [a3], 4
        .size   _main, . - _main

        .global _sub
        .type   _sub, @function
_sub:
        movm [a3], (sp)
        mov  sp, a3
L0:
        mov  _g_label, d1     # instruction is changed by relaxations
L1:
        mov  L1 - L0, d0      # the result of L1-L0 will become incorrect with -mrelax
        mov  a3, sp
        ret  [a3], 4
        .size   _sub, . - _sub

        .comm   _g_label,4,4
