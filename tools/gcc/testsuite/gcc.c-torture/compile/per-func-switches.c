/* { dg-require-effective-target trampolines } */
int  func2 (void) __attribute__((extra_switches ("-fcrossjumping")));
void func3 (int)  __attribute__((extra_switches ("-fcprop-registers")));

extern int a, b;

int func1 (void) { return a * 7 + b; }
int func2 (void) { return a * 17 + b; }

void func3 (int arg)
{
  struct s { int a; char b[arg]; };
  int bar (int (*)(struct s), int, void *);
  auto int nested (struct s) __attribute__((extra_switches ("-fargument-alias")));
  int nested (struct s x) { return x.a + sizeof (x); }
  struct s t;

  t.b[0] = 0;
  t.a = 123;
  bar (nested, arg, & t);
}
