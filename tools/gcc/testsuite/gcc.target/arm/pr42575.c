/* { dg-options "-O2" }  */
/* Make sure RA does good job allocating registers and avoids
   unnecessary moves.  */

long long longfunc(long long x, long long y)
{
      return x * y;
}
