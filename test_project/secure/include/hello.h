#include <string.h>
int get_string1(char buf[32]);
int get_string2(char buf[32]);

static int get_string3(char s[32]) {
  strncpy(s, "XXX, world!", 32);
  return 0;
}
