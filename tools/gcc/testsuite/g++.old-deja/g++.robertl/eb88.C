// { dg-do run  }
// { dg-require-effective-target size32plus }
// { dg-options "-w" }
// Another magic NULL problem.

#include <stddef.h>

int main()
{
  try
    {
      throw(NULL);
    }
  catch (...)
    {
    }
}
