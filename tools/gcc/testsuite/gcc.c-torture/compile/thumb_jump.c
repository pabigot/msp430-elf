/* This file is not part of the FSF GCC sources because it is an unstable
   test.  It is included here though because it may prove useful to Red Hat
   engineers hoping to track down similar bugs in other ports.  */
/* This file attempts to generate a branch instruction with an exact number
   of bytes between it and its destination, in order to check the branch-type
   selection logic in arm.md.  (ARM and THUMB branches have limited ranges
   depending upon their type).

   At the moment this file only tests for branches of +/- ~2048 bytes.  This
   may change in the future.

   The coding is dubious.  It relies upon the behaviour of gcc at the time of
   coding (2004-03-29) and will probably fail to produce the expected
   distances between the branches in the future.  The code has proved to be
   useful however in that it allowed a bug in arm.md to be isolated and
   removed.  */
   
int volatile * ptr;

#define M1     * p = b
#define M10    M1; M1; M1; M1; M1; M1; M1; M1; M1; M1; 
#define M100   M10; M10; M10; M10; M10; M10; M10; M10; M10; M10; 
#define M1000  M100; M100; M100; M100; M100; M100; M100; M100; M100; M100; 

void
test_backwards_by_2044 (void)
{
  int volatile * p = ptr;
  int b = 1;

 bar:
  /* Put 2044 bytes (= 1022 instructions) between the label and the goto.  */
  M1000;
  M10;
  M10;
  M1;
  M1;

  goto bar;
}

void
test_backwards_by_2046 (void)
{
  int volatile * p = ptr;
  int b = 1;

 bar:
  /* Put 2046 bytes (= 1023 instructions) between the label and the goto.  */
  M1000;
  M10;
  M10;
  M1;
  M1;
  M1;

  goto bar;
}

void
test_backwards_by_2048 (void)
{
  int volatile * p = ptr;
  int b = 1;

 bar:
  /* Put 2048 bytes (= 1024 instructions) between the label and the goto.  */
  M1000;
  M10;
  M10;
  M1;
  M1;
  M1;
  M1;

  goto bar;
}

void
test_forwards_by_2046 (void)
{
  int volatile * p;
  int b = 1;

  goto bar;
 baz:
  /* Put 2046 bytes (= 1023 instructions) between the goto and the label.  */
  M1000;
  M10;
  M10;
  M1;
  M1;
  M1;
 bar:
  if (* p)
    goto baz;
}

void
test_forwards_by_2048 (void)
{
  int volatile * p;
  int b = 1;

  goto bar;
 baz:
  /* Put 2048 bytes (= 1024 instructions) between the goto and the label.  */
  M1000;
  M10;
  M10;
  M1;
  M1;
  M1;
  M1;
 bar:
  if (* p)
    goto baz;
}

void
test_forwards_by_2050 (void)
{
  int volatile * p;
  int b = 1;

  goto bar;
 baz:
  /* Put 2050 bytes (= 1025 instructions) between the goto and the label.  */
  M1000;
  M10;
  M10;
  M1;
  M1;
  M1;
  M1;
  M1;
 bar:
  if (* p)
    goto baz;
}

void
test_forwards_by_2052 (void)
{
  int volatile * p;
  int b = 1;

  goto bar;
 baz:
  /* Put 2052 bytes (= 1026 instructions) between the goto and the label.  */
  M1000;
  M10;
  M10;
  M1;
  M1;
  M1;
  M1;
  M1;
  M1;
 bar:
  if (* p)
    goto baz;
}
