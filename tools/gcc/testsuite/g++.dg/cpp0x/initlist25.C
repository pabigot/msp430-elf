// PR c++/41754
// { dg-options -std=c++0x }
// { dg-do run }
// { dg-require-effective-target int32plus }

#include <map>
#include <string>
#include <iostream>

using namespace std;

int main()
{
        map<string, string> m;
        m.insert({{"t", "t"}, {"y", "y"}});

        return 0;
}
