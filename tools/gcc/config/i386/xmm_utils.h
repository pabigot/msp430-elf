/* RDHAT LOCAL whole file */

#ifndef _XMM_UTILS_H_INCLUDED
#define _XMM_UTILS_H_INCLUDED

#include <stdio.h>
#include "mmintrin.h"
#include "xmmintrin.h"

static __inline float _MM_FP (__m128 v, int t)
{
  union {
    __m128 v;
    float f[4];
  } u;
  u.v = v;
  return u.f[t];
}
static __inline int _MM_INT (__m128 v, int t)
{
  union {
    __m128 v;
    int i[4];
  } u;
  u.v = v;
  return u.i[t];
}

#define _MM_FP0(x) _MM_FP (v, 0)
#define _MM_FP1(x) _MM_FP (v, 1)
#define _MM_FP2(x) _MM_FP (v, 2)
#define _MM_FP3(x) _MM_FP (v, 3)
#define _MM_INT0(x) _MM_INT (v, 0)
#define _MM_INT1(x) _MM_INT (v, 1)
#define _MM_INT2(x) _MM_INT (v, 2)
#define _MM_INT3(x) _MM_INT (v, 3)

/* Provide functions that get around the restriction of requiring an
   immediate for some intrinsics as last argument.  */
#define __CASE1(X,Y,Z...) case X: return Y(Z, X);
#define __CASE2(X,Y,Z...) __CASE1(X,Y,Z) __CASE1(X+1,Y,Z)
#define __CASE4(X,Y,Z...) __CASE2(X,Y,Z) __CASE2(X+2,Y,Z)
#define __CASE8(X,Y,Z...) __CASE4(X,Y,Z) __CASE4(X+4,Y,Z)
#define __CASE16(X,Y,Z...) __CASE8(X,Y,Z) __CASE8(X+8,Y,Z)
#define __CASE32(X,Y,Z...) __CASE16(X,Y,Z) __CASE16(X+16,Y,Z)
#define __CASE64(X,Y,Z...) __CASE32(X,Y,Z) __CASE32(X+32,Y,Z)
#define __CASE128(X,Y,Z...) __CASE64(X,Y,Z) __CASE64(X+64,Y,Z)
#define __CASE256(X,Y,Z...) __CASE128(X,Y,Z) __CASE128(X+128,Y,Z)

static __inline __m128 _mmi_shuffle_hack(__m128 a, __m128 b, unsigned int imm8)
{
  __v4sf va = (__v4sf)a, vb = (__v4sf)b;
  switch (imm8 & 0xff)
    {
      __CASE256 (0, (__m128)__builtin_ia32_shufps, va, vb)
      default:
	/* Can't happen */
	return a;
    }
}


static __inline int _mmi_pextrw_hack(__m64 a, int imm8)
{
  __v4hi va = (__v4hi) a;
  switch (imm8 & 3)
    {
      __CASE4 (0, (__m64)__builtin_ia32_pextrw, va)
      default:
	/* Can't happen */
	return a;
    }
}

static __inline __m64 _mmi_pinsrw_hack(__m64 m, int w, int imm8)
{
  __v4hi va = (__v4hi) m;
  switch (imm8 & 3)
    {
      __CASE4 (0, (__m64)__builtin_ia32_pinsrw, va, w)
      default:
	/* Can't happen */
	return m;
    }
}

static __inline __m64 _mmi_pshufw_hack(__m64 m, int imm8)
{
  __v4hi tmp = (__v4hi) m;
  switch (imm8 & 0xff)
    {
      __CASE256 (0, (__m64)__builtin_ia32_pshufw, tmp)
      default:
	/* Can't happen */
	return m;
    }
}

/* Functions to print out vector values.  */
#define __MM_PRINT_FN(NAME, VECTYPE, TYPE, COUNT, EMMS, PRINTF)		\
static __inline void NAME (FILE *f, VECTYPE v)				\
{									\
  int i;								\
  union {								\
    VECTYPE vec;							\
    TYPE i[COUNT];							\
  } u;									\
  u.vec = v;								\
  EMMS									\
  fprintf (f, "[");							\
  for (i = 0; i < COUNT; i++)						\
    fprintf (f, " " PRINTF "%c", u.i[i], i + 1 == COUNT ? ' ' : ',');	\
  fprintf (f, "]\n");							\
}

__MM_PRINT_FN (_m_fprint_ps, __m128, float, 4, , "%f");
__MM_PRINT_FN (_m_fprint_mask, __m128, unsigned int, 4, , "%8x");

__MM_PRINT_FN (_m_fprintqi, __m64, int, 2, __builtin_ia32_emms();, "%d");
__MM_PRINT_FN (_m_fprintqw, __m64, short, 4,__builtin_ia32_emms();, "%d");
__MM_PRINT_FN (_m_fprintqb, __m64, signed char, 8, __builtin_ia32_emms();, "%d");
__MM_PRINT_FN (_m_fprintqui, __m64, unsigned int, 2, __builtin_ia32_emms();, "%u");
__MM_PRINT_FN (_m_fprintquw, __m64, unsigned short, 4, __builtin_ia32_emms();, "%u");
__MM_PRINT_FN (_m_fprintqub, __m64, unsigned char, 8, __builtin_ia32_emms();, "%u");

#define _mm_print_ps(V) _mm_fprint_ps(stdout, V)
#define _mm_print_mask(V) _mm_fprint_mask(stdout, V)
#define _m_printqi(V) _m_fprintqi(stdout, V)
#define _m_printqw(V) _m_fprintqw(stdout, V)
#define _m_printqb(V) _m_fprintqb(stdout, V)
#define _m_printqui(V) _m_fprintqui(stdout, V)
#define _m_printquw(V) _m_fprintquw(stdout, V)
#define _m_printqub(V) _m_fprintqub(stdout, V)

/* Functions to set MMX regs from integer values.  */

static __inline __m64 _m_setqi(int a, int b)
{
  union {
    __m64 vec;
    int i[2];
  } u;
  u.i[0] = a;
  u.i[1] = b;
  return u.vec;
}

static __inline __m64 _m_setqw(short a, short b, short c, short d)
{
  union {
    __m64 vec;
    short i[4];
  } u;
  u.i[0] = a;
  u.i[1] = b;
  u.i[2] = c;
  u.i[3] = d;
  return u.vec;
}

static __inline __m64 _m_setqb(char a, char b, char c, char d,
		      char e, char f, char g, char h)
{
  union {
    __m64 vec;
    char i[8];
  } u;
  u.i[0] = a;
  u.i[1] = b;
  u.i[2] = c;
  u.i[3] = d;
  u.i[4] = e;
  u.i[5] = f;
  u.i[6] = g;
  u.i[7] = h;
  return u.vec;
}

#define _m_setqui _m_setqi
#define _m_setquw _m_setqw
#define _m_setqub _m_setqb

static __inline __m64 _m_setqi2(int a)
{
  return _m_setqi (a, a);
}

static __inline __m64 _m_setqw4(short a)
{
  return _m_setqw (a, a, a, a);
}

static __inline __m64 _m_setqb8 (char a)
{
  return _m_setqb (a, a, a, a, a, a, a, a);
}

#define _m_setqui2 _m_setqi2
#define _m_setquw4 _m_setqw4
#define _m_setqub8 _m_setqb8

#endif
