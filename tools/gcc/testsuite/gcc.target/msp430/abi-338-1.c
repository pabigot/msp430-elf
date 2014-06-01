/* { dg-do compile } */

/* ABI check - 3.3.8 - two-byte struct arguments are aligned to a
   16-bit alignment on stack, regardless of the alignment requirements
   of its members.  */

typedef struct {
  char a;
  char b;
} S1;

extern void f1 (long, long, char, char, char, S1, char);

void
t1()
{
  static const S1 s1 = { 0x45, 0x67 };
/* { dg-final { scan-assembler-times "\.B.#0x1, @" 1 } } */
/* { dg-final { scan-assembler-times "\.B.#0x2, 0x1\\(" 1 } } */
/* { dg-final { scan-assembler-times "\.B.#0x3, 0x2\\(" 1 } } */
/* { dg-final { scan-assembler-times "\.B.#0x4, 0x" 1 } } */
  f1 (0x11112222L, 0x33334444L, 1, 2, 3, s1, 4);
}

