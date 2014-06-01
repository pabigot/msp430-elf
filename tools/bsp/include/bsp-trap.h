
#include "syscall.h"

#ifdef __MISSING_SYSCALL_NAMES__
#define _close close
#define _lseek lseek
#define _open open
#define _read read
#define _write write
#define _getpid getpid
#define _sbrk sbrk
#define _fstat fstat
#define _kill kill
#endif

/*
 *  Architecture specific trap mechanism.
 */
extern int _bsp_trap(int trap_nr, ...);

