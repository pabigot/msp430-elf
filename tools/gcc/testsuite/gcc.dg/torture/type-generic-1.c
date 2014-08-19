/* Do the type-generic tests.  Unlike pr28796-2.c, we test these
   without any fast-math flags.  */

/* { dg-do run } */
/* { dg-skip-if "No Inf/NaN support" { spu-*-* } } */
/* { dg-options "-DUNSAFE" { target tic6x*-*-* } } */
/* { dg-add-options ieee } */
/* { dg-xfail-run-if "default RX FP support is not IEEE compliant" { rx-*-* } { "*" } { "-m64bit-doubles" } } */

#include "../tg-tests.h"

int main(void)
{
  return main_tests ();
}
