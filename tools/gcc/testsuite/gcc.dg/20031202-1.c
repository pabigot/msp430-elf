/* { dg-do run } */
/* { dg-options "-O2" } */
/* { dg-options "-O2 -mtune=i686" { target { { i?86-*-* x86_64-*-* } && ia32 } } } */

extern void abort (void);
extern void exit (int);

struct A { char p[6]; } __attribute__((packed));
struct B {
    struct A a;
    void * const b;
    struct A const * const c;
    struct A const *d;
};

char v;

int __attribute__((noinline))
foo (struct B *b)
{
  int i;
  for (i = 0; i < 6; ++i)
    if (b->a.p[i])
      abort ();
  if (b->b != &v || b->c || b->d)
    abort ();
  return 12;
}

int __attribute__((noinline))
bar (void *x)
{
  struct B y = { .a.p[0] = 0, .a.p[1] = 0, .a.p[2] = 0, .a.p[3] = 0, .a.p[4] = 0, .a.p[5] = 0,
		 .b = x,
		 .c = (void *) 0,
		 .d = (void *) 0 };
  return foo (&y) + 1;
}

int
main (void)
{
  if (bar (&v) != 13)
    abort ();
  exit (0);
}
