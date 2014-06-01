// { dg-do run  }
// { dg-options "-O2" }
// { dg-require-effective-target int32plus }

#include <iostream>

std::ostream& foo (const char *x, std::ostream &y)
{
  return y << "" << x;
}

int main ()
{
  foo ("", std::cout);
}
