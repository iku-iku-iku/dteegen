#include "add.h"
#include <string>

namespace {
std::string s = "hello";
int add_internal(int x, int y) { return (x + y) % s.size(); }
} // namespace

int add(int x, int y) { return add_internal(x, y); }
