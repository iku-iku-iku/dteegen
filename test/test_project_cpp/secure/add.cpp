#include "add.h"
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
