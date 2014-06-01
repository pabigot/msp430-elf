// Test multi-level anonymous unions.  This was broken in some
// versions of g++ due to a bug in the test for an implicit
// typedef inside the anonymous union.
// { dg-do compile }
// { dg-require-effective-target int32plus }

int main (int argc, char *argv[])
{
     union
     {
         struct a
	 {
	    union
	    {
		struct b
		{
		    unsigned int c : 20;
		    unsigned int d : 12;
		} e;

		struct f
		{
		    unsigned int g : 22;
		    unsigned int h : 10;
		} i;
	    } /*anon*/;
	} k;
     } /*anon*/;
     
     k.i.h = 1;
     k.i.g = 2;
     k.e.d = 3;
     k.e.c = 4;
     return 0;
}
