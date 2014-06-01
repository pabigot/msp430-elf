/* { dg-do compile } */
/* { dg-options "-std=c99" } */

/* ABI check - 3.4 - return values are in registers R12 through 15,
   for up to 64-bit scalars.  Structs/unions bigger than 32 bits are
   returned by reference, with the address as a hidden first
   parameter.  */

char
t1()
{
/* { dg-final { scan-assembler-times "MOV\.W.#18, R10" 1 } } */
  return 0x12;
}

int
t2()
{
/* { dg-final { scan-assembler-times "MOV.W.#4660, R10" 1 } } */
  return 0x1234;
}

long
t3()
{
/* { dg-final { scan-assembler-times "MOV.W.#22136, R" 2 } } */
/* { dg-final { scan-assembler-times "MOV.W.#4660, R" 3 } } */
  return 0x12345678L;
}

long long
t4()
{
/* { dg-final { scan-assembler-times "MOV.W.#-8464, R." 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#-25924, R." 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#22136, R." 2 } } */
/* { dg-final { scan-assembler-times "MOV.W.#4660, R." 3 } } */
  return 0x123456789abcdef0LL;
}

typedef struct
{
  int a;
} S1;

typedef struct
{
  int a;
  int b;
} S2;

typedef struct
{
  int a;
  int b;
  int c;
} S3;

S1
t5 (void)
{
  register S1 s1 = { 1 };
/* { dg-final { scan-assembler-times "MOV.W.#1, R" 1 } } */
  return s1;
}

S2
t6 (void)
{
  register S2 s2 = { 2, 3 };
/* { dg-final { scan-assembler-times "MOV.W.#2, R" 1 } } */
/* { dg-final { scan-assembler-times "MOV.W.#3, R" 1 } } */
  return s2;
}

S3
t7 (void)
{
  register S3 s3 = { 4, 5, 6 };
  /* R12 is the hidden first parameter, which is the address of the
     buffer to use for the return value.  */
/* { dg-final { scan-assembler-times ".W.#4, .\\(R1" 1 } } */
/* { dg-final { scan-assembler-times ".W.#5, .\\(R1" 1 } } */
/* { dg-final { scan-assembler-times ".W.#6, .\\(R1" 1 } } */
  return s3;
}
