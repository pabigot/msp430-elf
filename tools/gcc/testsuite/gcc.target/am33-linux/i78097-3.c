/* { dg-do compile } */
/* { dg-options "-mam33-2" } */
/* { dg-final { scan-assembler "fadd" } } */

float foo(float a, float b)
{
  return a + b;
}
