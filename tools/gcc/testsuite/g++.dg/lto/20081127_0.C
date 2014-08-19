// { dg-require-effective-target size32plus }
struct Foo { Foo(); };
static void func() { new Foo(); }
