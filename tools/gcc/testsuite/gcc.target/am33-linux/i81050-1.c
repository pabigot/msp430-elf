/* { dg-do run } */

#include <stdio.h>
#include <math.h>
#include <complex.h>

static float complex xx(void)
{
 return (float complex)2;
}

#define VAR_SZ (sizeof(float complex)/sizeof(long))

void sub1()
{
 union {
   float complex c;
   long l[VAR_SZ];
 } v;
 int i;

 v.c = xx();
 i = (int)crealf(v.c);
 if (i != 2)
   {
     printf("i = %d\n", i);
     abort();
   }
}

int main()
{
  int i = (int)crealf(xx());
  if (i != 2)
   {
     printf("i = %d\n", i);
     abort();
   }
 sub1();
 return 0;
}
