#include "../secure/add.h"
#include <iostream>

extern "C" int get_num() { return 42; }

int main(int argc, char *argv[]) {
  /* std::cout << add(1, 2) << std::endl; */
  /**/
  /* std::cout << addf(1.1, 2.2) << std::endl; */

  std::cout << pi() << std::endl;
  std::cout << get_e() << std::endl;
  return 0;
}
