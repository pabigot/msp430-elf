/* { dg-do run } */

#include <stdio.h>

void ll_check(int n, long long i, long long j)
{
   if( i != j )
     abort();
}

int main()
{
  unsigned long long i = 5;
  unsigned long long j = 2;

  j = ~j;
  ll_check(__LINE__, i>>~j, 1);
  ll_check(__LINE__, i<<~j, 20); j = ~j;

  ll_check(__LINE__, i^~j, ~(unsigned long long)7);
  ll_check(__LINE__, i|~j, ~(unsigned long long)2);
  ll_check(__LINE__, i?~j:0, ~(unsigned long long)2);
  ll_check(__LINE__, i=~j, ~(unsigned long long)2);

  return 0;
}
