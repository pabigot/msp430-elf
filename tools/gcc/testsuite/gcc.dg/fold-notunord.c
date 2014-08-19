/* { dg-do compile } */
/* { dg-options "-O -ftrapping-math -fdump-tree-optimized" } */
/* { dg-skip-if "test does not match RX behaviour" { "rx-*-*" } { "*" } { "" } } */

int f (double d)
{
  return !__builtin_isnan (d);
}

/* { dg-final { scan-tree-dump " ord " "optimized" } } */
/* { dg-final { cleanup-tree-dump "optimized" } } */
