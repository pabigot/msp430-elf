/* PR inline-asm/56405 */
/* { dg-skip-if "inline assembler not supported" { rl78-*-* } { "*" } { "" } } */

void
foo (void)
{
  asm volatile ("" : "+m" (*(volatile unsigned short *) 0x1001UL));
}
