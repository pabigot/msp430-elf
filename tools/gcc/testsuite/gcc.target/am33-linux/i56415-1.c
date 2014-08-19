/* { dg-do run } */

int main()
{
  struct {
    unsigned int b1: 1;
    unsigned int b2: 2;
    unsigned int b3: 3;
    unsigned int b4: 4;
    unsigned int b5: 5;
    unsigned int b6: 6;
    unsigned int b7: 7;
    unsigned int b8: 8;
  } B;

  B.b1 = 1;
  B.b2 = 2;
  B.b7 = 7;
  B.b8 = 8;

  if (B.b1 != 1)
    {
      printf("B.b1 = %d\n", B.b1);
      abort();
    }
  if (B.b2 != 2)
    {
      printf("B.b2 = %d\n", B.b2);
      abort();
    }
  exit(0);
}
