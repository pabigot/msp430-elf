/* { dg-lto-do link } */
// { dg-require-effective-target size32plus }

class foo {
 public:
 foo ();
 virtual ~foo ();
};

foo::foo ()
{
}

int
main ()
{
 foo dummy;
 return 0;
}
