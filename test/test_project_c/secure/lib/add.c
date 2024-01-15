#include "some_math.h"

static int secret = 5;

int add(int x, int y) {
  int a = qpow(x, y);
  int b = qpow(y, x);
  return (a + b) % secret;
}
