#include <string.h>
extern "C" {
#define TA_HELLO_WORLD "secgear hello world!"
int get_string(char buf[32]) {
  strncpy(buf, TA_HELLO_WORLD, strlen(TA_HELLO_WORLD) + 1);
  return 0;
}
}
