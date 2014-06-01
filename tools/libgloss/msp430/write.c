#include <string.h>

#include "cio.h"

static int
write_chunk (int fd, char *buf, int len)
{
  __CIOBUF__.length[0] = len;
  __CIOBUF__.length[1] = len >> 8;
  __CIOBUF__.parms[0] = CIO_WRITE;
  __CIOBUF__.parms[1] = fd;
  __CIOBUF__.parms[2] = fd >> 8;
  __CIOBUF__.parms[3] = len;
  __CIOBUF__.parms[4] = len >> 8;
  memcpy (__CIOBUF__.buf, buf, len);

  _libgloss_cio_hook ();

  return __CIOBUF__.parms[0] + __CIOBUF__.parms[1] * 256;
}

#include <stdio.h>

int
write (int fd, char *buf, int len)
{
  int rv = 0;
  int c;
#if 0
  if (fd == 2)
    fprintf (stderr, "%.*s", buf, len);
  else if (fd == 1)
    printf ("%.*s", buf, len);
#endif
  while (len > 0)
    {
      int l = (len > CIO_BUF_SIZE) ? CIO_BUF_SIZE : len;
      c = write_chunk (fd, buf, l);
      if (c < 0)
	return c;
      rv += l;
      len -= l;
      buf += l;
    }
  return c;
}
