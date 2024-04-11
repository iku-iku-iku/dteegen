#include "../secure/add.h"
#include "TEE-Capability/dtee_sdk.h"
int main() {

  auto ctx = init_distributed_tee_context({.side = SIDE::Server});
  publish_secure_function(ctx, add_x_and_y);
  publish_secure_function(ctx, mul_x_and_y);
  dtee_server_run(ctx);
  destroy_distributed_tee_context(ctx);
}
