#include "strcat.h"
int concat_string(char s1[32], char s2[32], char dest[64]) {
  int d_i = 0;
  for (int i = 0; i < 32 && s1[i]; ++i) {
    dest[d_i++] = s1[i];
  }
  for (int i = 0; i < 32 && s2[i]; ++i) {
    dest[d_i++] = s2[i];
  }
  dest[d_i] = '\0';
  return 0;
}
