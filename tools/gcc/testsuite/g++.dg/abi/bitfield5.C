// { dg-do compile } 
// { dg-require-effective-target non_ms_bitfield_layout }
// { dg-options "-Wabi -fabi-version=1" }
// { dg-options "-Wabi -fabi-version=1 -mno-ms-bitfields" { target i?86-*-* x86_64-*-* } }

struct A { 
  virtual void f(); 
  int f1 : 1; 
};

struct B : public A {
  int f2 : 1;  // { dg-warning "ABI" "" { xfail rx-*-* } }
  int : 0;
  int f3 : 4; 
  int f4 : 3;
};
