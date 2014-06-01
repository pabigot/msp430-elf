/* Test static local optimization. */
/* { dg-do compile } */

extern int z;
extern int external_func (void);

/* Do not convert statics that are not local to a function.  */
static int n = 0;

int
can1 (void)
{
  static int can_be_made_auto = 0;

  can_be_made_auto = 3;

  return can_be_made_auto ++;
}

void
can2 (int *x)
{
  static int can_be_made_auto;

  for (can_be_made_auto = 0; can_be_made_auto < n; can_be_made_auto ++)
    x[can_be_made_auto] = 0;

  n = z;
}

void
maybe1 (int *x, int len)
{
  static int could_be_shadowed_by_auto = 0;

  while (could_be_shadowed_by_auto < len)
    x [could_be_shadowed_by_auto ++] = 0;
}

int
maybe2 (void)
{
  static int could_be_made_auto = 0;

  if (external_func ())
    return 1;
  
  could_be_made_auto = 3;

  return could_be_made_auto ++;
}

int
maybe3 (int arg)
{
  static int must_be_static;
  static int can_be_auto;

  can_be_auto = must_be_static;
  must_be_static = z;

  return can_be_auto + must_be_static;
}

/* Do not convert a static which can be used without having been initialized first.  */

int
not1 (int arg)
{
  static int must_be_static;

  if (arg > z)
    must_be_static = 2;

  return must_be_static;
}

int
not2 (int arg)
{
  static int must_be_static;
  static int can_be_auto;

  can_be_auto = must_be_static;
  must_be_static = z;

  return can_be_auto + must_be_static;
}

int
not3 (int function_arg)
{
  static int must_be_static = 0;
  int already_auto = must_be_static;
  
  must_be_static = function_arg;
  return already_auto + must_be_static;
}

int
not4 (int arg)
{
  if (external_func ())
    {
      static int must_be_static = 0;
  	
      if (arg > 10)
	must_be_static = 1;
      else
	must_be_static ++;
  
      return must_be_static;
    }
  else
    return arg + 1;
}

#ifndef __STRICT_ANSI__
/* Do not convert local statics that are used inside nested functions.  */

int
not5 (int arg)
{
  static int must_be_static = 1;

  int nested (void) { return must_be_static * external_func (); }

  if (arg)
    return nested ();
  else
    return must_be_static = arg;
}
#endif

/* Do not convert a local static whose address is taken.  */

int *
not6 (void)
{
  static int x;
  x = 0;
  external_func ();
  return & x;
}

/* Here is a subtle one.  Two mutually recursive functions with a local static variable.
   In the first version (foo_can), the static is initialized every time the function is
   entered, and so it can be converted.  In the second version (foo_not) the static is
   only ever initialized once (at program load) and so it cannot be converted.  */

     int bar_can (int);
     int bar_not (int);

     int foo_can (int i) { static int s; s = 0; if (bar_can (i)) s = 1; return s; }
     int foo_not (int i) { static int s = 0;    if (bar_not (i)) s = 1; return s; }
     int bar_can (int i) { if (i) { foo_can (i - 1); return 0; } return 1; }
     int bar_not (int i) { if (i) { foo_not (i - 1); return 0; } return 1; }

/* Even if the static is initialized every time the function is invoked, it
   still cannot be converted to a local if it is subject to recursion.  For
   example:
   
     int
     foo (int a)
     {
       static int x;
       int b;

       x = a;
       if (a == 0)
         return 0;
       b = foo (a - 1);
       return x + b;
     }

   Change the "static int x" to just "int x" in the above program and you
   will get completely different results.  For example foo(3) will give
   0 with a "static int x" and 6 with an "int x".

   Hence this optimization can only safely be applied to leaf functions, which
   at the time of writing are undetectable at the point when the optimization
   is run.  */
