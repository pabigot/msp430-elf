int foo(void)
{
  static int x asm ("z") = 3;
  return x++;
}

int X2 asm ("z.0") = 4;
int X3 asm ("_z.0") = 5;

