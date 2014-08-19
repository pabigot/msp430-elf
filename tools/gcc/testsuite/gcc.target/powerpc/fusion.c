/* { dg-do compile { target { powerpc*-*-* } } } */
/* { dg-skip-if "" { powerpc*-*-darwin* } { "*" } { "" } } */
/* { dg-skip-if "" { powerpc*le-*-* } { "*" } { "" } } */
/* { dg-require-effective-target powerpc_p8vector_ok } */
/* { dg-options "-mcpu=power7 -mtune=power8 -O3" } */

#define LARGE 0x12345

int fusion_uchar (unsigned char *p){ return p[LARGE]; }
int fusion_schar (signed char *p){ return p[LARGE]; }
int fusion_ushort (unsigned short *p){ return p[LARGE]; }
int fusion_short (short *p){ return p[LARGE]; }
int fusion_int (int *p){ return p[LARGE]; }
unsigned fusion_uns (unsigned *p){ return p[LARGE]; }

double fusion_vector (double *p) { return p[2]; }

