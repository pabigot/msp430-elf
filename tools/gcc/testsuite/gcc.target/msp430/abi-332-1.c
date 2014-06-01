/* { dg-do compile } */

/* ABI check - 3.3.2 - 32-bit arguments are passed in (unaligned)
   register pairs.  Includes "long int", "float", and structs up to 32
   bit in size.  */

typedef struct {
  short a;
  char b;
  char c;
} S1;

extern void f1 (long);
extern void f2 (float);
extern void f3 (S1);

extern void f4 (long, long);
extern void f5 (long, long, long);

extern void f6 (int, long);
extern void f7 (int, int, long);

void
t1 ()
{
/* { dg-final { scan-assembler-times "MOV.W.#0x1212, R13" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#0x1313, R12" 1 } } */
  f1 (0x12121313L);
}

void
t2 ()
{
/* { dg-final { scan-assembler-times "MOV.W.#0x3fa6, R13" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#0x6666, R12" 1 } } */
  f2 (1.3);
}

void
t3 ()
{
  static S1 s1 = { 0x1122, 0x33, 0x44 };
/* { dg-final { scan-assembler-times "MOV.*s1.*, R12" 1 } } */
  f3 (s1);
}
