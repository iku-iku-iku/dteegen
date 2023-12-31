#include "add.h"
#include "hello.h"
#include <stdio.h>

int main() {
  char buf[32];
  get_string1(buf);
  printf("The result is %s\n", buf);

  printf("add(3, 4) = %d\n", add(3, 4));

  get_string2(buf);
  printf("The result is %s\n", buf);
}
