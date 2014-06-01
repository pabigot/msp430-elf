/* { dg-do compile } */
/* { dg-options "-mno-mem-funcs" } */
/* { dg-final { scan-assembler "memcpy" } } */

/* Make sure that the compiler generates at least one call to memcpy
   even though the -mno-mem-funcs command line option has been specified.  */

extern int memcpy (char *, const char *, int);
extern int mem_check (char *);

static char buff[64];

int
memcpy_test (void)
{
  int   len = 0;

  memcpy (buff, "3_Z", 3);
  len += mem_check (buff);

  memcpy (buff, "19_P_p_j_J_c_C_d_MZ", 19);
  len += mem_check (buff);

  return len;
}
