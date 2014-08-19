/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-optimized" } */

#ifdef __hppa__
#define REGISTER "1"
#elif defined __RL78__
#define REGISTER "r8"
#else
#ifdef __moxie__
#define REGISTER "2"
#else
#define REGISTER "0"
#endif
#endif

void baz(void)
{
	register int xyzzy asm(REGISTER) = 1;
	asm volatile ("" : : "r"(xyzzy));
}

/* { dg-final { scan-tree-dump-times "asm\[^\\r\\n\]*xyzzy" 1 "optimized" } } */
/* { dg-final { cleanup-tree-dump "optimized" } } */
