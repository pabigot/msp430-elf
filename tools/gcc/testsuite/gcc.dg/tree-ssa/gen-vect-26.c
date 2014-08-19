/* { dg-do run { target vect_cmdline_needed } } */
/* { dg-options "-O2 -ftree-vectorize -fdump-tree-vect-details -fvect-cost-model=dynamic" } */
/* { dg-options "-O2 -ftree-vectorize -fdump-tree-vect-details -fvect-cost-model=dynamic -mno-sse" { target { i?86-*-* x86_64-*-* } } } */

#include <stdlib.h>

#define N 128

/* unaligned store.  */

int main ()
{
  int i;
  char ia[N+1];

  for (i = 1; i <= N; i++)
    {
      ia[i] = 5;
    }

  /* check results:  */
  for (i = 1; i <= N; i++)
    {
      if (ia[i] != 5)
        abort ();
    }

  return 0;
}

/* { dg-final { cleanup-tree-dump "vect" } } */
