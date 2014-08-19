/* { dg-do run } */
/* { dg-options "-fsigned-char" } */

void
i_check(int i, int j, int k)
{
  if( j != k )
    abort();
}

void
l_check(int i, long j, long k)
{
  if( j != k )
    abort();
}

int
main()
{
  unsigned int       ui;
  long               l;
  char               c, *pc;
  unsigned char      uc, *puc;
  float              f, *pf;

  pc = &c;
  puc = &uc;
  pf = &f;

  ui = 23;
  *pf = 16;
  i_check(__LINE__, ui -= *pf, 7);

  l = 14;
  *pc = 7;
  *pc -= l;
  i_check(__LINE__, *pc, -7);

  *puc = 21;
  l -= *puc;
  l_check(__LINE__, l, -7L);

  l = 14;
  l_check(__LINE__, l -= *puc, -7L);

  exit(0);
}
