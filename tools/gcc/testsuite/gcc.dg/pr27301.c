/* PR c/27301 */
/* { dg-do compile } */
/* { dg-skip-if "asm construct does not work with RL78" { rl78-*-* } { "*" } { "" } } */
/* { dg-options "-O2 -std=gnu89" } */

void
foo (void *ptr, long n)
{
  __asm__ __volatile__ ("" :: "m" (({ struct { char x[n]; } *p = ptr; *p; })));
}

void
bar (void *ptr, long n)
{
  __asm__ __volatile__ ("" :: "m" (*({ struct { char x[n]; } *p = ptr; p; })));
}
