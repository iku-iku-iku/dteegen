#include "img_receiver.h"

int img_receiver(std::array<char, IMG_SIZE> arr) {
  char emb[EMBEDDING_SIZE];
  int res = embedding(arr.data(), emb);
  // the embedding is sealed and can be stored safely.
  return res;
}
