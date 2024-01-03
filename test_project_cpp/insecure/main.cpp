#include "../secure/hello.h"
#include <iostream>

int main() {
  char buf[32];
  get_string(buf);
  std::cout << buf << std::endl;
}
