// Test that we have one EH cleanup region for the whole array
// rather than one for each element.
// { dg-options "-fdump-tree-gimple" }

struct A
{
  A();
  ~A();
};

void f()
{
  A a[10] = { };
}

// { dg-final { cleanup-tree-dump "gimple" } }
