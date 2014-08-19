/* { dg-do run } */
/* { dg-require-effective-target trampolines } */
/* { dg-options "-pg" } */
/* { dg-options "-pg -static" { target hppa*-*-hpux* } } */
/* { dg-require-profiling "-pg" } */
/* { dg-skip-if "no profiling support" { "msp430-*-*" } { "*" } { "" } } */

extern void abort(void);

void foo(int i)
{
  void bar(void)
  {
    if (i != 2)
      abort ();
  }

  bar();
}

int main(void)
{
  foo (2);
  return 0;
}

/* { dg-final { cleanup-profile-file } } */
