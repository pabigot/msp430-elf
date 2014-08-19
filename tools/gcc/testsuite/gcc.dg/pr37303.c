/* { dg-do compile { target *-*-elf* *-*-gnu* } } */
/* { dg-skip-if "RX backend does not produce expected output" { "rx-*-*" } { "*" } { "" } } */
/* { dg-options "-std=c99" }
/* { dg-final { scan-assembler "rdata|rodata" } } */

struct S { const int *x; } s = { (const int[]){1, 2, 3} };
