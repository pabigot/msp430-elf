/* { dg-do compile { target { powerpc*-*-* } } } */
/* { dg-skip-if "" { powerpc*-*-darwin* } { "*" } { "" } } */
/* { dg-require-effective-target powerpc_p8vector_ok } */
/* { dg-options "-O2 -mcpu=power8" } */
/* { dg-final { scan-assembler-not "\[ \t\]and "     } } */
/* { dg-final { scan-assembler-not "\[ \t\]or "      } } */
/* { dg-final { scan-assembler-not "\[ \t\]xor "     } } */
/* { dg-final { scan-assembler-not "\[ \t\]nor "     } } */
/* { dg-final { scan-assembler-not "\[ \t\]eqv "     } } */
/* { dg-final { scan-assembler-not "\[ \t\]andc "    } } */
/* { dg-final { scan-assembler-not "\[ \t\]orc "     } } */
/* { dg-final { scan-assembler-not "\[ \t\]nand "    } } */

#ifndef TYPE
typedef int v4si __attribute__ ((vector_size (16)));
#define TYPE v4si
#endif

#include "bool2.h"
