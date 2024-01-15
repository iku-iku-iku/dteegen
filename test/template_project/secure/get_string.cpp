#include "get_string.h"
const char *secret = "hello wolrd";
int get_string(char s[32]) {
  int i = 0;
  while (i < 31 && secret[i] != '\0') {
    s[i] = secret[i];
    i++;
  }
  s[i] = 0;
  return 0;
}
