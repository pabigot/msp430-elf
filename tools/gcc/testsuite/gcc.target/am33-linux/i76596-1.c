/* { dg-do compile { xfail *-*-* } } */

int
t()
{
  return 1;
}
    
static int
val_size(int i, float a[5][i]) /* { dg-error "prior" "" { xfail *-*-* } } */
{
  return sizeof a[0]; /* { dg-error "confused" "" { xfail *-*-* } } */
}
    
void
sub_f()
{
  float a[5][3];
  val_size(3, a);
}
