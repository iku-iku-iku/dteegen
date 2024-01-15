#include "../secure/get_string.h"
#include <iostream>

int main() {
  char s[32];
  get_string(s);
  std::cout << s << std::endl;
  return 0;
}
