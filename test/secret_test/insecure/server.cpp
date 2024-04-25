
#include "../secure/receive.h"
#include "TEE-Capability/dtee_sdk.h"

int main() {
  auto ctx = init_distributed_tee_context(
      {.side = SIDE::Server, .mode = MODE::ComputeNode});
  //auto ctx = init_distributed_tee_context({.side = SIDE::Server});
  //publish_secure_function(ctx, add);
  //publish_secure_function(ctx, mul);
  dtee_server_run(ctx);
}