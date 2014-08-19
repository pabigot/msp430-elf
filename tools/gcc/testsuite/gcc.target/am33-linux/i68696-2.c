/* { dg-do compile } */
/* { dg-options "-mmem-funcs" } */
/* { dg-final { scan-assembler "memcpy" } } */
/* { dg-final { scan-assembler "memset" } } */

/* The original test case from Issue 68696.  */

int
main (void)
{
  char a[10] = {0};
  char b[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  return 0;
}

/* An extended version which will generate calls to
   both memset and memcpy, even at high levels of
   optimisation.  */

typedef struct
{
  int a[25];
} S;

extern int bar (S *);


int
foo (int arg)
{
  S a = {0};
  char b[10] = {11, 0x44, 0, 99, 0x99, 6, 55, 1, 9, 100};

  bar (& a);

  return b [arg];
}
