/* { dg-do compile { target { nonpic } } } */
/* { dg-options "-fipa-pta -fdump-ipa-pta" } */
/* { dg-skip-if "" { *-*-* } { "-O0" "-fno-fat-lto-objects" } { "" } } */

struct X { char x; char y; };

char *q;

static void __attribute__((noinline))
bar (char *p)
{
  q = p;
}

void test1 (char a1, char b, char c, char d, char e, char f, char g, char h)
{
  char *p = &a1;
  p++;
  bar (p);
}

void test2 (struct X a2, char b, char c, char d, char e, char f, char g, char h)
{
  char *p = &a2.x;
  p++;
  bar (p);
}

void test3 (struct X a3, char b, char c, char d, char e, char f, char g, char h)
{
  char *p = &a3.y;
  bar (p);
}

void test4 (int a4, char b, char c, char d, char e, char f, char g, char h)
{
  char *p = (char *)&a4;
  p++;
  p++;
  p++;
  p++;
  bar (p);
}

// MSP430 puts the funtion address into a register so it does not appear
// repeated (as much as expected) in the assembler.
// { dg-final { global target_triplet } }
// { dg-final { if [string match msp430-*-* $target_triplet ] {return} } }
/* { dg-final { scan-ipa-dump "bar.arg0 = { test4.arg0 test3.arg0 test2.arg0 test1.arg0 }" "pta" } } */
/* { dg-final { cleanup-ipa-dump "pta" } } */
