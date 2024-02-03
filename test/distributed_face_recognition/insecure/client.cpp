#include "TEE-Capability/distributed_tee.h"
#include "img_receiver.h"

int main() {
  auto ctx = init_distributed_tee_context({.side = SIDE::Client,
                                           .mode = MODE::Transparent,
                                           .name = "face_recognition",
                                           .version = "1.0"});
  /* std::array<char, IMG_SIZE> arr; */
  /* for (int i = 0; i < IMG_SIZE; i++) { */
  /*   arr[i] = 1; */
  /* } */
  char arr[IMG_SIZE];
  for (int i = 0; i < IMG_SIZE; i++) {
    arr[i] = 1;
  }
  char res[EMBEDDING_SIZE];
  int sealed_data_len = embedding(arr, res);
  printf("sealed_data_len: %d\n", sealed_data_len);

  /* int res = call_remote_secure_function(ctx, img_receiver, arr); */
  /* std::cout << res << std::endl; */
  destroy_distributed_tee_context(ctx);
}
