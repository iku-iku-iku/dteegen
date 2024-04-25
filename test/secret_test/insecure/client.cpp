
#include "../secure/receive.h"
#include "TEE-Capability/distributed_tee.h"

int main() {
  auto ctx = init_distributed_tee_context({.side = SIDE::Client,
                                           .mode = MODE::Transparent,
                                           .name = "secret_test",
                                           .version = "1.0"});

  receive((char*)"My password is 123456");
  destroy_distributed_tee_context(ctx);
}
