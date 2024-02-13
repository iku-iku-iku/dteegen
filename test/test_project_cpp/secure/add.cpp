#include "add.h"
#include "../insecure/num.h"
#include <string>

extern "C" void eapp_print(const char *, ...);
namespace {
class Object {
public:
  Object(std::string s) : s(std::move(s)) { eapp_print("Object created\n"); }
  std::size_t len() { return s.size(); }

private:
  std::string s;
};
static Object s("hello");
int add_internal(int x, int y) {
  eapp_print("s.len: %d\n", s.len());

  return (x + y) % s.len();
}
} // namespace

int add(int x, int y) { return add_internal(x, y); }

float addf(float x, float y) { return (x + y) * get_num(); }

void print_num(int num) { eapp_print("%d", num); }
bool check_nan(float f) {
  unsigned int *bits = (unsigned int *)&f;

  // NaN的特点是指数域全为1，尾数域不全为0
  return (((*bits) & 0x7F800000) == 0x7F800000) &&
         (((*bits) & 0x007FFFFF) != 0);
}
void print_float(float num) {
  // check num is nan
  if (check_nan(num)) {
    eapp_print("nan");
    return;
  }
  print_num((int)(num * 10000));
}
float pi() {
  float res = 0;
  int a = 1;
  for (int i = 0; i < 100000000; i++) {
    res += 4.0f / (float)a;
    a += 2;
    res -= 4.0f / (float)a;
    a += 2;
  }
  return res;
}
float get_e() {
  float res = 0;
  float a = 1;
  for (int i = 1; i < 100000000; i++) {
    res += 1 / a;
    a *= i;
  }
  return res;
}
