/* { dg-lto-do link } */
// { dg-require-effective-target size32plus }

class C {
 public:
  C();
  virtual ~C();
  virtual void foo();
};
void bar() {
  new C();
}

C::C() {

}

C::~C() {

}

void C::foo() {
}

int main(void)
{
  return 0;
}
