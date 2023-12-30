#include "hello.h"
#include <stdio.h>

int main() {
  char buf[32];
  get_string1(buf);
  printf("The result is %s\n", buf);

  printf("1 + 1 = %d\n", add(1, 1));

  get_string2(buf);
  printf("The result is %s\n", buf);
}
