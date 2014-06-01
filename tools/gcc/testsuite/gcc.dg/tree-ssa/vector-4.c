/* { dg-do compile } */
/* { dg-options "-w -O1 -fdump-tree-gimple" } */
/* On 16-bit targets vector expansion does not work in the way this test expects.  */
/* { dg-require-effective-target int32plus } */

typedef int v4si __attribute__ ((vector_size (16)));

v4si vs (v4si a, v4si b)
{
  return __builtin_shuffle (a, b, (v4si) {0, 4, 1, 5});
}

/* The compound literal should be placed directly in the vec_perm.  */
/* Test is xfailed on 32-bit hppa*-*-* because target-callee-copies.  */
/* { dg-final { scan-tree-dump-times "VEC_PERM_EXPR <a, b, { 0, 4, 1, 5 }>;" 1 "gimple" { xfail { hppa*-*-* && { ! lp64 } } } } } */

/* { dg-final { cleanup-tree-dump "gimple" } } */
