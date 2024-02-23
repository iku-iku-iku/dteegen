
#include "../secure/lua_rt.h"
#include "TEE-Capability/dtee_sdk.h"
#include "run_proxy.hpp"

int main() {
  /* auto ctx = init_distributed_tee_context( */
  /*     {.side = SIDE::Server, .mode = MODE::ComputeNode}); */
  auto ctx = init_distributed_tee_context({.side = SIDE::Server});
  publish_secure_function(ctx, add);
  publish_secure_function(ctx, mul);
  publish_secure_function(ctx, run_proxy);
  dtee_server_run(ctx);
}
