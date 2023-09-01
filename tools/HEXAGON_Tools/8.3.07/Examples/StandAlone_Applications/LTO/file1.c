#include "file1.h"

static signed int i = 0;

void foo2(void) {
  i = -1;
}

__attribute__((noinline))
static int foo3() {
  foo4();
  return 10;
}

int foo1(void) {
  int data = 0;

  if (i < 0)
    data = foo3();

  data = data + 42;
  return data;
}
