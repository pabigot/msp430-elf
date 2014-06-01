// { dg-do compile }

// Ignore warning on some powerpc-linux configurations.
// { dg-prune-output "non-standard ABI extension" }
#define vector __attribute__((vector_size(16) ))
vector unsigned long int f(int a)
{
  vector unsigned long int mask = a ? (vector unsigned long int){ 0x80000000, 0x80000000,
0x80000000, 0x80000000 } : (vector unsigned long int){0};
  return mask;
}

