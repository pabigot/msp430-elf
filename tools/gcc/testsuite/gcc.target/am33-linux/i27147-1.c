/* { dg-do compile } */
/* { dg-final { scan-assembler "fcmp" } } */

int func( float f )
{
 return (f==0)? 1:0;
}
