/* Test promotion of __fp16 to double as arguments to unprototyped
   function in another compilation unit.  */

/* { dg-do compile } */
/* { dg-options "-mfp16-format=ieee" } */


#include <stdlib.h>

#include "fp16-unprototyped-2.c"

extern int f ();

static __fp16 x = 42.0;
static __fp16 y = -42.0;

int
main (void)
{
  if (!f (x, y))
    abort ();
  return 0;
}
