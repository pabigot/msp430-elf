extern int printf (const char *, ...);
#if 0
#include <stdio.h>
#endif

void
foo (void)
{
  printf ("MAIN\n");
}

asm (".symver foo,foo@FOO");
