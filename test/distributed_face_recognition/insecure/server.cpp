#include "img_receiver.h"
#include "TEE-Capability/distributed_tee.h"

int main() {
  auto ctx = init_distributed_tee_context({.side = SIDE::Server});
  publish_secure_function(ctx, img_receiver);
  tee_server_run(ctx);
}
