#include "img_receiver.h"
#include <filesystem>
#include <fstream>
#include <regex>

#define INF 1000000

constexpr float THRESHOLD = 0.6;

int img_recorder(std::array<char, IMG_SIZE> arr, int id) {

  for (int i = 0; i < 5; i++) {
    printf("img[%d] %d\n", i, (int)arr[i]);
  }
  for (int i = IMG_SIZE - 5; i < IMG_SIZE; i++) {
    printf("img[%d] %d\n", i, (int)arr[i]);
  }
  char emb[EMBEDDING_SIZE];
  int sealed_data_len = embedding(arr.data(), emb);
  float *ff = (float *)emb;
  // for (int i = 0; i < 10; i++) {
  //   printf("emb[%d] %f\n", i, ff[i]);
  // }
  // the embedding is sealed and can be stored safely.

  std::string filename = "emb" + std::to_string(id) + ".bin";

  std::ofstream out(filename, std::ios::binary);
  out.write(emb, sealed_data_len);
  return sealed_data_len;
}

int img_verifier(std::array<char, IMG_SIZE> arr) {
  char emb1[EMBEDDING_SIZE];
  char emb2[EMBEDDING_SIZE];

  char in_emb[EMBEDDING_SIZE];
  int sealed_data_len = embedding(arr.data(), in_emb);
  float *ff = (float *)in_emb;
  // for (int i = 0; i < 10; i++) {
  //   printf("emb[%d] %f\n", i, ff[i]);
  // }

  float min_dist = INF;
  int min_dist_id = -1;
  for (const auto &e : std::filesystem::directory_iterator(".")) {
    const auto &path = e.path();
    const auto path_str = path.string();
    // if path_str belike "emb*.bin", take its id and do a comparison in enclave
    std::regex re("emb(\\d+)\\.bin");
    if (std::regex_search(path_str, re)) {
      std::smatch match;
      std::regex_search(path_str, match, re);
      int id = std::stoi(match[1]);
      std::ifstream in(path_str, std::ios::binary);
      in.read(emb1, EMBEDDING_SIZE);

      memcpy(emb2, in_emb, EMBEDDING_SIZE);
      float dist;
      // calculate_distance(emb1, emb2, (char *)&dist);
      dist = calculate_distance(emb1, emb2);
      printf("DISTANCE WITH PERSON%d: %f\n", id, dist);

      if (dist < min_dist) {
        min_dist = dist;
        min_dist_id = id;
      }
    }
  }

  if (min_dist < THRESHOLD * THRESHOLD) {
    return min_dist_id;
  }
  return -1;
}
