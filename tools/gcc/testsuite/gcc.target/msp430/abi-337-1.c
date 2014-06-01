/* { dg-do compile } */

/* ABI check - 3.3.7 - structs and unions larger than 32 bits are
   passed by reference, with the address passed as a 16-bit pointer in
   its place.  */

typedef struct {
  int a, b, c, d, e, f;
} S1;

typedef struct {
  int a, b;
} S2;

extern void f1 (int, S1, int);
extern void f2 (int, S2, int);

void
t1()
{
  static S1 s1;
/* { dg-final { scan-assembler-times "MOV.W.#0x1111, R12" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#0x2222, R14" 1 } } */
  f1 (0x1111, s1, 0x2222);
}

void
t2()
{
  static S2 s2;
/* { dg-final { scan-assembler-times "MOV.W.#0x3333, R12" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#0x4444, R1" 1 } } */
  f2 (0x3333, s2, 0x4444);
}
