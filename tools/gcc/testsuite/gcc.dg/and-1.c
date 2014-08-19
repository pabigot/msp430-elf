/* { dg-do compile } */
/* { dg-options "-O2" } */

int f(int y)
{
  return y & ~(y & -y);
}
