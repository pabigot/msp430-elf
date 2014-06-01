#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trap.h"

int isatty (int) __attribute__ ((weak, alias ("_isatty")));

_isatty (fd)
     int fd;
{
  return 1;
}
