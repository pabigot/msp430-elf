/* { dg-do compile } */
/* { dg-options "-std=c99" } */

/* ABI check - 3.3.4 - 64-bit arguments are either passed in R12-R15,
   or entirely on the stack.  */

extern void f1 (long long);
extern void f2 (int, long long);

void
t1 ()
{
/* { dg-final { scan-assembler-times "MOV.W.#-8464, R" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#-25924, R" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#22136, R" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#4660, R" 1 } } */
  f1 (0x123456789abcdef0LL);
}

void
t2 ()
{
/* { dg-final { scan-assembler-times ".W.#-8464, @" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#-25924, .*\\(" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#22136, .*\\(" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#4660, .*\\(" 1 } } */
  f2 (1, 0x123456789abcdef0LL);
}
