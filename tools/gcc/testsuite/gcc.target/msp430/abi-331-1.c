/* { dg-do compile } */
/* { dg-final { scan-assembler-times "MOV.W.#0xa, R12" 5 } } */
/* { dg-final { scan-assembler-times "MOV.W.#0x14, R13" 4 } } */
/* { dg-final { scan-assembler-times "MOV.W.#0x1e, R14" 3 } } */
/* { dg-final { scan-assembler-times "MOV.W.#0x28, R15" 2 } } */
/* { dg-final { scan-assembler-times ".W.#0x32, @R1" 1 } } */
/* ABI check - 3.3.1 - up to four arguments are passed in registers, if
   they fit.  Argument registers are R12 through R15.  */

extern void f1 (int);
extern void f2 (int, int);
extern void f3 (int, int, int);
extern void f4 (int, int, int, int);
extern void f5 (int, int, int, int, int);

void
t1 ()
{
  f1 (10);
}

void
t2 ()
{
  f2 (10, 20);
}

void
t3 ()
{
  f3 (10, 20, 30);
}

void
t4 ()
{
  f4 (10, 20, 30, 40);
}

void
t5 ()
{
  f5 (10, 20, 30, 40, 50);
}
