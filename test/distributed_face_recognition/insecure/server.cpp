#include "TEE-Capability/dtee_sdk.h"
#include "img_receiver.h"

int main() {
  /* auto ctx = init_distributed_tee_context( */
  /*     {.side = SIDE::Server, .mode = MODE::ComputeNode}); */
  /* dtee_server_run(ctx); */
  auto ctx = init_distributed_tee_context({.side = SIDE::Server});
  publish_secure_function(ctx, img_recorder);
  publish_secure_function(ctx, img_verifier);
  tee_server_run(ctx);
}
