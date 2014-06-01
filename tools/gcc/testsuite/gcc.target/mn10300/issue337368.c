/* { dg-do assemble } */

/* This file used to compile to bogus assembler code when optimization
   was enabled.  The copy propagation pass would change the test of
   flag.field_4, from using an address register to using the stack
   pointer, which is invalid for a BTST instruction.  */

#include <string.h>

typedef struct
{
  _Bool field_1 : 1;
  _Bool field_2 : 1;
  _Bool field_3 : 1;
  _Bool field_4 : 1;
} BitField_t;

void
sample (unsigned short data, unsigned short * p)
{
  BitField_t flag;

  memset (& flag, 0, sizeof (BitField_t));

  if (flag.field_4 == 0x0)
    {
      *p = data;
    }
}
