/* { dg-do compile } */

/* ABI check - 3.3.8 - Funtions with "..." parameters pass the last
   named parameter and all following parameters on the stack.  */

extern void f1 (int, ...);
extern void f2 (int, int, ...);
extern void f3 (int, int, int, int, ...);
extern void f4 (int, int, int, int, int, ...);

void
t1()
{

/* { dg-final { scan-assembler-not "MOV.W.#4660, R" } } */
  f1 (0x1111, 0x2222, 0x3333, 0x4444, 0x5555);
}

void
t2()
{
/* { dg-final { scan-assembler-times "MOV.W.#0x1234, R12" 1 } } */
/* { dg-final { scan-assembler-not "MOV.W.#0x2222, R" } } */
  f2 (0x1234, 0x2222, 0x3333, 0x4444, 0x5555);
}


void
t3()
{
/* { dg-final { scan-assembler-not "MOV.W.#4, R" } } */
  f3 (1, 2, 3, 4, 0x2468, 6, 7, 8);
}


void
t4()
{
/* { dg-final { scan-assembler-not "MOV.W.#0x1357, R" } } */
  f4 (10, 11, 12, 13, 0x1357, 16);
}

