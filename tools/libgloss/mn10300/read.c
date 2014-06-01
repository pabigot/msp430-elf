#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trap.h"

int read (int, char *, size_t) __attribute__ ((weak, alias ("_read")));

_read (int file,
       char *ptr,
       size_t len)
{
  return TRAP0 (SYS_read, file, ptr, len);
}
