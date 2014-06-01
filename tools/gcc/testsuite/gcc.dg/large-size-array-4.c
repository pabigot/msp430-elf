/* PR c/25309 */
/* { dg-do compile } */
/* { dg-options "" } */
/* { dg-require-effective-target ilp32 } */
static char * name[] = {
    [0x80000000]  = "bar"
  };
/* { dg-error "too large" "" { target { { ! lp64 } && { ! llp64 } } }  6 } */
