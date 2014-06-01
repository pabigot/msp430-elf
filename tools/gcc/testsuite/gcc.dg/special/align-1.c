/* { dg-require-effective-target ilp32 } */
/* This test is intended to test the aligned() attribute for large
   alignments.  Large values will not be supported by all targets,
   since it relies upon support from the object file forma.  In
   particular non-ELF based systems may not be able handle these
   alignments.

   A variable is declared before each instance of the structure in
   order to check that the structure itself is created on a proerly
   aligned boundary.  The b, c and d fields inside the structure
   also have non-natural alignments, in ordet to check that they get
   padded properly.
   
   If the test of the structure in the common domain fails, it may
   be because the target does not define ASM_OUTPUT_ALIGNED_COMMON
   or ARM_OUTPUT_ALIGNED_DECL_COMMON macros.  This was the cause
   of the original inspiration of this test.  */
extern void abort (void);
extern void exit (int);
typedef struct align
{
  unsigned long a[1];
  unsigned long b[1] __attribute__ ((aligned (8)));
  unsigned long c[1] __attribute__ ((aligned (16)));
  unsigned long d[1] __attribute__ ((aligned (32)));
} ALIGN;

char a;
ALIGN common;

char b __attribute__ ((section (".bss")));
ALIGN bss __attribute__ ((section (".bss")));

char c = 1;
ALIGN data = {1, 2, 3, 4};

int
main (void)
{
  if ((((unsigned long) common.b) & 0x00000007) || 
      (((unsigned long) common.c) & 0x0000000f) ||
      (((unsigned long) common.d) & 0x0000001f))
    abort ();

  if ((((unsigned long) bss.b) & 0x00000007) || 
      (((unsigned long) bss.c) & 0x0000000f) ||
      (((unsigned long) bss.d) & 0x0000001f))
    abort ();

  if ((((unsigned long) data.b) & 0x00000007) || 
      (((unsigned long) data.c) & 0x0000000f) ||
      (((unsigned long) data.d) & 0x0000001f))
    abort ();

  exit (0);
}
