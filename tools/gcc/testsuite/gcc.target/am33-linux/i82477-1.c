/* { dg-options "-fpie" } */
/* { dg-do compile } */
/* { dg-error "-fpie is not supported" "" { target am33*-*-* } 1 } */
void
sub()
{
}
