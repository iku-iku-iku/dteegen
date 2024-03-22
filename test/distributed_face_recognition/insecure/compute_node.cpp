#include "TEE-Capability/dtee_sdk.h"
#include "file.h"
int main() {
  (void)write_file;
  (void)get_emb_list;
  (void)read_file;
  auto ctx = init_distributed_tee_context(
      {.side = SIDE::Server, .mode = MODE::ComputeNode});
  dtee_server_run(ctx);
  destroy_distributed_tee_context(ctx);
}
