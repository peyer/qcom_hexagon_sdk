// clang -c --compile-and-analyze
// Report1: Potential leak of memory pointed to by 'p'
// Report2: Address of stack memory associated with local variable 'i'
//  is still referred to by the global variable 'gp' upon returning 
//  to the caller. This will be a dangling reference
unsigned int *gp;
int foo(unsigned int argc) {
  int* p = new int[10];
  unsigned int i = 100;
  gp = &i;
  if (argc>*p)
    return i;
  return *p;
}
