// { dg-do run  }
int main ()
{
  const int *p = new const int (0);
  delete p;
}
// { dg-require-effective-target size32plus }
