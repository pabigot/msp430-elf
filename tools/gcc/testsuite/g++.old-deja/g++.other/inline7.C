// { dg-do run  }
// { dg-require-effective-target size32plus }
// { dg-options "-O2" }
// Origin: Mark Mitchell <mark@codesourcery.com>

#include <list>

std::list<int*> li;

void f ()
{
  li.size ();
}

int main ()
{
  li.push_back (0);
  f ();
}
