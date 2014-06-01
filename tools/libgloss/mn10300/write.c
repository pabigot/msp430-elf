#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "trap.h"

int write (int, char *, size_t) __attribute__ ((weak, alias ("_write")));

int
_write ( int file,
	 char *ptr,
	 size_t len)
{
  return TRAP0 (SYS_write, file, ptr, len);
}
