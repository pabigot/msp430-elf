// { dg-lto-do link }
// { dg-lto-options {{-O3 -flto -Winline}} }
// { dg-require-effective-target size32plus }

#include <string>

int
main()
{
  std::string i;
  i = "abc";
}

