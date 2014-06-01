/* ISO C99 7.6: Floating-point environment.  */

#ifndef _FENV_H
#define _FENV_H	1

/* This file just provides a very basic set of ISO C99 conforming declarations
   and functions.  Its real purpose is to provide a default header in case the
   target does not provide its own version.  */

typedef int fenv_t;
typedef int fexcept_t;

#define FE_INVALID	(1 << 0)
#define FE_DIVBYZERO	(1 << 1)
#define FE_OVERFLOW	(1 << 2)
#define FE_UNDERFLOW	(1 << 3)
#define FE_INEXACT	(1 << 4)

#define FE_ALL_EXCEPT 	(FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

#define FE_TONEAREST	1
#define FE_DOWNWARD	2
#define FE_UPWARD	3
#define FE_TOWARDZERO	4

#define FE_DFL_ENV      ((const fenv_t *) -1)

static int feclearexcept (int);
static int fegetexceptflag (fexcept_t *, int);
static int feraiseexcept (int);
static int fesetexceptflag (const fexcept_t *, int);
static int fetestexcept (int);
static int fegetround (void);
static int fesetround (int);
static int fegetenv (fenv_t *);
static int feholdexcept (fenv_t *);
static int fesetenv (const fenv_t *);
static int feupdateenv (const fenv_t *);

static inline int feclearexcept (int e) { return 0; }
static inline int fegetexceptflag (fexcept_t * p, int e) { return 0; }
static inline int feraiseexcept (int e) { return 0; }
static inline int fesetexceptflag (const fexcept_t * p, int e) { return 0; }
static inline int fegetround (void) { return -1; }
static inline int fesetround (int dir) { return -1; }
static inline int fegetenv (fenv_t * arg) { if (arg == 0) return -1; * arg = 0; return 0; }
static inline int feholdexcept (fenv_t * arg) { return -1; }
static inline int fesetenv (const fenv_t * arg) { return -1; }
static inline int feupdateenv (const fenv_t * arg) { return -1; }

#endif /* _FENV_H */
