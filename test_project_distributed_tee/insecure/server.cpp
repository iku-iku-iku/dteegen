#include "../secure/add.h"
#include "TEE-Capability/distributed_tee.h"

int main() {
  auto ctx = init_distributed_tee_context({.side = SIDE::Server});
  publish_secure_function(ctx, add);
  tee_server_run(ctx);
}
