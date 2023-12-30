#include "hello.h"
#include <stdio.h>

int main() {
  char buf[32];
  get_string(buf);
  printf("The result is %s\n", buf);
}
