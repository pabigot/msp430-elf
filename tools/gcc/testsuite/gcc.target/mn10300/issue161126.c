/* { dg-do run } */

extern int abort (void);

struct st_sub 
{
 char str[10];
};

struct st
{
 struct st_sub * memp;
 int             index;
};

struct st_sub array[10];
struct st XX;

int
main (void) 
{
 XX.memp = array;

 XX.index = 0;

 XX.memp[XX.index++] = array[1];

 if (XX.index != 1)
   abort ();

 return 0;
}
