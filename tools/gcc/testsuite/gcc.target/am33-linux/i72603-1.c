/* { dg-do run } */

#include<stdio.h>

struct {
  long long v;
  const char *fmt;
  const char *result;
} tests[] = {
  { 9223372036854775807LL, "%lld", "9223372036854775807" },
  { 9223372036854775807LL, "%llx", "7fffffffffffffff" },
  { 0LL, "%lld", "0" },
  { 0LL, "%llx", "0" },
  { -1LL, "%lld", "-1" },
  { -1LL, "%llx", "ffffffffffffffff" },
  { -9223372036854775807LL, "%lld", "-9223372036854775807" },
  { -9223372036854775807LL, "%llx", "8000000000000001" },
  { 0, 0, 0 }
};

int main(viod)
{
  int i;
  int err = 0;
  char buf[100];

  for (i=0; tests[i].fmt; i++)
    {
      sprintf(buf, tests[i].fmt, tests[i].v);
      if (strcmp (tests[i].result, buf))
	{
	  printf("test %d exp %s got %s\n", i, tests[i].result, buf);
	  err ++;
	}
    }
  return err;
}
