/* { dg-require-effective-target ptr32plus } */
typedef void  *(*T)(void);
f1 ()
{
  ((T) 0)();
}
f2 ()
{
  ((T) 1000)();
}
f3 ()
{
  ((T) 100000)();
}
f4 (r)
{
  ((T) r)();
}
f5 ()
{
  int (*r)() = f3;
  ((T) r)();
}