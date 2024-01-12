#include "../secure/add.h"
#include "distributed_tee.h"

int main() {
  auto ctx = init_distributed_tee_context({.side = SIDE::Client});
  int res = call_remote_secure_function(ctx, add, 1, 2);
  std::cout << res << std::endl;
}
