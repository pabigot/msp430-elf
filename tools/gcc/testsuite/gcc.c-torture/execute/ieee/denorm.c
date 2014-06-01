/* Test 32-bit floating point operations on denormal numbers.  */

extern void abort (void);
extern void exit (int);
extern int printf (const char *, ...);

/* XXX: FIXME: Before contributing to the FSF:
 * Add a big-endian version.  */

typedef union
{
  float        fval;
  struct
  {
#if (__SIZEOF_INT__ >= 4)
    unsigned int fraction:23 __attribute__((packed));
#else
    unsigned long fraction:23 __attribute__((packed));
#endif
    unsigned int exp:8 __attribute__((packed));
    unsigned int sign:1 __attribute__((packed));
  } bits;
}
float_bits_union;

float f0       = 0.0;
float minus_f0 = -0.0;
float f2       = 2.0;

/* The smallest possible normal 32-bit floating point value.  */
volatile float_bits_union n = { .bits.fraction = 1, .bits.exp = 1, .bits.sign = 0 };
/* Negative version of n.  */
volatile float_bits_union minus_n = { .bits.fraction = 1, .bits.exp = 1, .bits.sign = 1 };
/* The smallest possible sub-normal 32-bit floating point value.  */
volatile float_bits_union d1 = { .bits.fraction = 1, .bits.exp = 0, .bits.sign = 0 };
/* d1 * 2  */
volatile float_bits_union d2 = { .bits.fraction = 2, .bits.exp = 0, .bits.sign = 0 };
/* d3 is a copy of d1.  */
volatile float_bits_union d3 = { .bits.fraction = 1, .bits.exp = 0, .bits.sign = 0 };


int
main (void)
{
  unsigned int fails = 0;
  float zero;
  float two;
  float normal;
  float denormal;

  if (sizeof (float) != 4)
    exit (0);

  /* Get the values via globals so that gcc cannot eliminate the tests below.  */
  zero = f0;
  two =  f2;
  normal = n.fval;
  denormal = d1.fval;

  /* Check that zero division works normally.  */
  if (zero / normal != zero)
    ++ fails;

  /* The AM33/2.0 FPU gets this wrong.  The denormalized input value
     is treated as 0, generating NaN as a result, instead of 0.  */
  if (zero / denormal != zero)
    ++ fails;

  /* The AM33/2.0 FPU gets this wrong.  The result should be
     a denormalized number, but the AM33/2.0 FPU returns +0.0.  */
  if (normal / two == zero)
    ++ fails;

  /* The AM33/2.0 FPU gets this wrong.  The result should be
     a denormalized number, but the AM33/2.0 FPU returns -0.0.  */
  if (minus_n.fval / two == minus_f0)
    ++ fails;

  if ((denormal * two != d2.fval)
      || (two * d3.fval == zero))
    ++ fails;

  if ((denormal + d3.fval != d2.fval)
      || (d3.fval + d3.fval == zero))
    ++ fails;

  if ((d2.fval - denormal != denormal)
      || (d2.fval - denormal == zero))
    ++ fails;

  if ( - denormal >= zero)
    ++ fails;

  if (fails)
    abort ();
  exit (0);
}
