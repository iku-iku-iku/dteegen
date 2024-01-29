#include "../secure/add.h"
#include "TEE-Capability/distributed_tee.h"

int main() {
  auto ctx = init_distributed_tee_context(
      {.side = SIDE::Client, .mode = MODE::MIGRATE, .name = "template_client"});
  int res;
  int a = 1, b = 2;
  res = call_remote_secure_function(ctx, mul, 1, 2);
  printf("mul(%d, %d) == %d\n", a, b, res);
  res = call_remote_secure_function(ctx, add, 1, 2);
  printf("add(%d, %d) == %d\n", a, b, res);
}
