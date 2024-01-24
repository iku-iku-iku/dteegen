#include "TEE-Capability/distributed_tee.h"
#include "img_receiver.h"

int main() {
  auto ctx = init_distributed_tee_context({.side = SIDE::Client});
  std::array<char, IMG_SIZE> arr;
  for (int i = 0; i < IMG_SIZE; i++) {
    arr[i] = 1;
  }

  int res = call_remote_secure_function(ctx, img_receiver, arr);
  std::cout << res << std::endl;
}
