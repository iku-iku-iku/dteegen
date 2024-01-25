#include "../secure/seal.h"
#include <iostream>

const char *plain_text = "Hello, world!";

int main() {
  char s[64];
  seal_data((char *)plain_text, s);

  unseal_data(s, s);

  printf("%s\n", s);
  return 0;
}
