// PR tree-optimization/42625
// { dg-do run }
// { dg-options "-O1 -fipa-sra" }
// { dg-additional-sources "dtor4-aux.cc" }

#include "dtor4.h"

int
main ()
{
  S s;
  return 0;
}
// { dg-require-effective-target size32plus }
// { dg-require-effective-target size32plus }
