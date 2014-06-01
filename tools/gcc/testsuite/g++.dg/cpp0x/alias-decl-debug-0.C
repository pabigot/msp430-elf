// Origin: PR c++/51032
// { dg-skip-if "No stabs" { mmix-*-* *-*-aix* alpha*-*-* hppa*64*-*-* ia64-*-* *-*-vxworks* } { "*" } { "" } }
// { dg-skip-if "No stabs" { v850-*-* } { "*" } { "" } }
// { dg-options "-std=c++0x -gstabs+" }

template <class C>
struct A {
    template<class U> using B = U*;
    int a;
};

A<int> a;

