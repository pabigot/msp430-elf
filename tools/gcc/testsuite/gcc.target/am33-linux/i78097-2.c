/* { dg-do compile } */
/* { dg-options "-mam33-2" } */

#if __AM33__ != 2
#error __AM33__ != 2
#endif

#ifndef __AM33_2__
#error __AM33_2__ not defined
#endif

main()
{
  exit(0);
}
