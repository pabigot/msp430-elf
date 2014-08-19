/* { dg-do compile } */
// { dg-require-effective-target int32plus }
/* { dg-options "-Wstrict-aliasing=2 -O2" } */

float foo ()
{
  unsigned int MASK = 0x80000000;
  return (float &) MASK; /* { dg-warning "strict-aliasing" } */
}

