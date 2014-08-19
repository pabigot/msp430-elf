// PR c++/50424
// { dg-do run }

int f() { throw 1; }
void g( int = f() ) { }
void h() { g(); }
int main()
{
  try { h(); } catch (int) { }
}
// { dg-require-effective-target size32plus }
// { dg-require-effective-target size32plus }
