extern int printf (const char *, ...);
#if 0
#include <stdio.h>
#endif

extern void foo (void);

void
foo (void)
{
  printf ("DSO\n");
}

void
bar (void)
{
  foo ();
}
