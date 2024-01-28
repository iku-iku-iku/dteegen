#include "../secure/add.h"
#include "TEE-Capability/dtee_sdk.h"

int main() {
  auto ctx = init_distributed_tee_context(
      {.side = SIDE::Server, .mode = MODE::COMPUTE_NODE});
  dtee_server_run(ctx);
}
