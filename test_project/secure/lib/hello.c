#include "../../insecure/strcat.h"
#include <string.h>
#define TA_HELLO_WORLD "secgear hello world!"
#define TA_BYEBYE_WORLD "byebye "

int get_strlen(const char *str) {
  int len = 0;
  for (; *str; str++) {
    len++;
  }
  return len;
}

int get_string1(char buf[32]) {
  strncpy(buf, TA_HELLO_WORLD, get_strlen(TA_HELLO_WORLD) + 1);
  return 0;
}

int get_string2(char buf[32]) {
  concat_string(TA_BYEBYE_WORLD, "world!", buf);
  return 0;
}
