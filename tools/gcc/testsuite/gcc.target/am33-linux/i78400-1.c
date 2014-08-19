/* { dg-do compile { xfail *-*-* } } */
/* { dg-options "--std=c99" } */

#include <stddef.h>

const wchar_t str[] = L"\u4E16\u754C";
const int \u4E16 = 10;		/* { dg-error "stray" } */
const int \u0048 = 123;		/* { dg-error "stray" } */
const char * a = "\u0048";	/* { dg-error "not a valid" } */
