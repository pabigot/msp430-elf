// PR c++/34918
// { dg-do compile }

long int v __attribute ((vector_size (8)));
bool b = !(v - v);	// { dg-error "could not convert .\\(__vector.2. long int\\)\\{0l, 0l\\}. from .__vector.2. long int. to .bool.|in argument to unary" }
