// { dg-lto-do run }
// { dg-require-effective-target size32plus }

static const char *fname;
struct S
{
  S () { fname = __func__; }
};
extern "C" void abort (void);
int
main ()
{
  S tmp;
  if (fname[0] != 'S')
    abort ();
  return 0;
}
