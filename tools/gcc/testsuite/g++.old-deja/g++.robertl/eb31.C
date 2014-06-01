// { dg-do run  }
// { dg-require-effective-target int32plus }
#include<iostream>

int main() {
  try {
    throw 1;
  } catch(...) {
   try {
     throw;
   } catch(int) {
   }
   try {
     throw;
   } catch(int) {
   }
  }
  return 0;
}


