/* { dg-do compile } */
/* { dg-final { scan-assembler-not "fadd" { xfail *-*-* } } } */

float foo(float a, float b)
{
  return a + b;
}
