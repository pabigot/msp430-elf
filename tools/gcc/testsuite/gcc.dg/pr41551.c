/* { dg-do compile } */
/* { dg-options "-O2" } */
/* { dg-require-effective-target ilp32 } */

/* Make sure we do not ICE.  */

__extension__ typedef __UINTPTR_TYPE__ uintptr_t;

int main(void)
{
 int var, *p = &var;
 return (double)(uintptr_t)(p);
}
