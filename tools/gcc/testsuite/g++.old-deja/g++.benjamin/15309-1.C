// { dg-do assemble  }
// { dg-options "-Wnon-virtual-dtor -Weffc++" }
// 981203 bkoz
// g++/15309

class bahamian {
public:
  bahamian ();
  ~bahamian ();  
};

class miami : public bahamian
{
public:
   miami ();
   ~miami ();
};




