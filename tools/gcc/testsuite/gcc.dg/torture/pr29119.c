/* { dg-do compile } */
/* { dg-skip-if "asm construct does not work with RL78" { rl78-*-* } { "*" } { "" } } */

void ldt_add_entry(void)
{
   __asm__ ("" :: "m"(({unsigned __v; __v;})));
}

