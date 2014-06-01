/* PR inline-asm/15740 */
/* { dg-do compile } */
/* { dg-options "-O" } */
/* { dg-skip-if "asm construct does not work with RL78" { rl78-*-* } { "*" } { "" } } */

void foo(void)
{
  int a, b;
  a = 1;
  b = a + 1;
  asm ("" : : "m" (a));
}
