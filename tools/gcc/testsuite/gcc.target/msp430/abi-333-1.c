/* { dg-do compile } */

/* ABI check - 3.3.3 - 32-bit values may be split between R15 and the stack */

extern void f1 (int, int, int, long);

void
t1()
{
/* { dg-final { scan-assembler-times "MOV.W.#0x5678, R15" 1 } } */
/* { dg-final { scan-assembler-times ".W.#0x1234, @" 1 } } */
  f1 (1, 2, 3, 0x12345678);
}
