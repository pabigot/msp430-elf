/* { dg-do compile } */
/* { dg-options "-mam33" } */

#if __AM33__ == 2
#error __AM33__ == 2
#endif

#if __AM33_2__ == 2
#error __AM33_2__ == 2
#endif

main()
{
  exit(0);
}
