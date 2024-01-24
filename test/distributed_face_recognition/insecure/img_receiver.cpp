#include "img_receiver.h"

int img_receiver(std::array<char, IMG_SIZE> arr) {
  char emb[EMBEDDING_SIZE];
  int res = embedding(arr.data(), emb);

  float *embedding_float = (float *)emb;
  int len = EMBEDDING_SIZE / sizeof(float);
  for (int i = 0; i < len; i++) {
    std::cout << embedding_float[i] << " ";
  }
  std::cout << std::endl;
  return res;
}
