/* { dg-skip-if "" { pdp11-*-* } { "*" } { "" } } */
/* PR target/35318 */
/* { dg-skip-if "Too many registers needed on 16-bit targets" { "xstormy16-*-*" } { "*" } { "" } } */

void
foo ()
{
  double x = 4, y;
  __asm__ volatile ("" : "=r,r" (x), "=r,r" (y) : "%0,0" (x), "m,r" (8));
}
