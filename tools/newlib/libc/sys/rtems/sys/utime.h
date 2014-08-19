/*
 *  $Id: utime.h,v 1.10 2014/02/20 13:50:17 nickc Exp $
 */

#ifndef __UTIME_h__
#define __UTIME_h__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  POSIX 1003.1b 5.6.6 Set File Access and Modification Times
 */

struct utimbuf {
  time_t  actime;   /* Access time */
  time_t  modtime;  /* Modification time */
};

/* Functions */

int utime(
  const char           *path,
  const struct utimbuf *times
);

#ifdef __cplusplus
};
#endif

#endif /* _SYS_UTIME_H */
