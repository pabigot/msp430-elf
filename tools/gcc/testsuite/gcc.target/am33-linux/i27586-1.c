/* { dg-do compile } */
/* { dg-final { scan-assembler "cmp" } } */

int x;

int func(int a)
{
       a = (a + 3) & ~3;

       while (a > 0) {
             a -= x;
       }
       return a;
}

main()
{
  
}
