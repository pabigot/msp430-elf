// { dg-do run  }
// { dg-require-effective-target int32plus }
// 981203 bkoz
// g++/15071
// gcc invocation fails to link in libstdc++

#include <iostream>

int main() {
  std::cout << "hi" << std::endl;

  return 0;
}
