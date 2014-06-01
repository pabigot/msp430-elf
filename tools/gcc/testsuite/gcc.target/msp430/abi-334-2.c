/* { dg-do compile } */
/* { dg-options "-std=c99" } */

/* ABI check - 3.3.4 - Once a 64-bit value is put on the stack,
   "split" 32-bit values are not allowed, and put entirely on the
   stack.  16-bit values still use a register.  */

extern void f1 (int, long long, long); /* not split */
extern void f2 (int, int, int, long long, long); /* split not allowed */

void
t1 ()
{
/* { dg-final { scan-assembler-times "MOV.W.#4951, R." 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#9320, R." 1 } } */
  f1 (0x1111, 0x123456789abcdef0LL, 0x24681357L);
}

void
t2 ()
{
/* { dg-final { scan-assembler-not "MOV.W.#9319, R" } } */
/* { dg-final { scan-assembler-not "MOV.W.#4952, R" } } */
  f2 (0x1111, 0x2222, 0x3333, 0x123456789abcdef0LL, 0x24671358L);
}
