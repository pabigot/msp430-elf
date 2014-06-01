t1()
{
  int foo[5];
  bar(foo);

  __asm ("; clobber R5" ::: "R5");
}
