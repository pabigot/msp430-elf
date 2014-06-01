/* Contributed by Jürgen Keil <jrgn.keil@googlemail.com> */

/* { dg-do run } */
/* { dg-require-profiling "-pg" } */
/* { dg-options "-O2 -pg" } */
/* { dg-options "-O2 -pg -static" { target hppa*-*-hpux* } } */
/* { dg-skip-if "no profiling support" { "msp430-*-*" } { "*" } { "" } } */

extern char *strdup (const char *);

void
func(char *a, char *b, char *c)
{
  strdup(a);
  strdup(b);
  strdup(c);
}

int
main(void)
{
  func("a", "b", "c");
  return 0;
}

/* { dg-final { cleanup-profile-file } } */
