/* { dg-require-effective-target trampolines } */

int
main (int argc, char **argv)
{
  int size = 10;

  typedef struct
  {
    char val[size];
  } block;

  block
    retframe_block (void)
    {
      return *(block*)0;
    }

  return 0;
}
