#include "cio.h"

__CIOBUF__TYPE__ __CIOBUF__;

void
_libgloss_cio_hook (void)
{
  __asm__ __volatile__ (" .global C$$IO$$");
  __asm__ __volatile__ ("C$$IO$$: nop");
}
