#include <stdio.h>
#include <string.h>
#define TA_HELLO_WORLD "secgear hello world!"
#define TA_BYEBYE_WORLD "secgear byebye world!"

static int secret = 5;

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

int add(int x, int y) { return (x + y) % secret; }

int get_string2(char buf[32]) {
  strncpy(buf, TA_BYEBYE_WORLD, get_strlen(TA_BYEBYE_WORLD) + 1);
  return 0;
}
