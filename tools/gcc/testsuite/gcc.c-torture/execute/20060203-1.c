void
cmp (int n, int x, int y)
{
  if (x != y)
    abort ();
}

int
sub (void)
{
  static int array_i[3] = {0,1,2};
  signed char array_c[12];
  short array_s[12];
    
  struct
  {
    unsigned int b1: 1;
    unsigned int b2: 8;
#if (__SIZEOF_INT__ >= 4)
    unsigned int b3: 32;
#else
    unsigned long b3: 32;
#endif
  } BF;

  BF.b1 = 1;
  array_c[array_i[1]] = 8;
  array_c[array_i[1]] |= BF.b1;
  cmp(__LINE__, array_c[array_i[1]], 9);
  cmp(__LINE__, array_c[array_i[1]] |= BF.b1, 9);

  BF.b1 |= 1;
  array_s[2] = 9;
  array_s[2] |= BF.b1;
  cmp(__LINE__, array_s[2], 9);

  array_s[2] = 9;
  cmp(__LINE__, array_s[2] |= BF.b1, 9);

  return BF.b1;
}

int
main (void)
{
  (void) sub ();
  exit (0);
}
