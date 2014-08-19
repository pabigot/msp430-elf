/* { dg-do run } */

#include <stdio.h>

int
main()
{
  int     *x;

  x = &(int){10};
  if (*x != 10)
    abort();
  exit(0);
}
