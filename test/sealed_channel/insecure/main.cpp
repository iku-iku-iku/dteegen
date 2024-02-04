#include "../secure/sealed_channel_test.h"
#include "receiver.h"
#include <TEE-Capability/distributed_tee.h>

extern "C" int sender(char *buf, int buf_len) {
  DistributedTeeConfig config{.side = SIDE::Client,
                              .mode = MODE::Normal,
                              .name = "sealed_channel_client",
                              .version = "1.0"};
  auto ctx = init_distributed_tee_context(config);
  std::vector<char> data(buf, buf + buf_len);
  auto res = call_remote_secure_function(ctx, receiver, data);
  printf("REAL_SEND\n");
  return res;
}
int main(int argc, char **argv) {
  (void)sender;
  if (argc != 2) {
    printf("Usage: %s <send/receive>\n", argv[0]);
  }

  if (argv[1][0] == 's') {
    send();
  }

  if (argv[1][0] == 'r') {
    DistributedTeeConfig config{.side = SIDE::Server,
                                .mode = MODE::Normal,
                                .name = "sealed_channel_server",
                                .version = "1.0"};
    auto ctx = init_distributed_tee_context(config);
    char buf[BUF_LEN];
    publish_secure_function(ctx, receiver);
    tee_server_run(ctx);
  }
  return 0;
}
