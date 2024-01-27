#include "../secure/seal.h"
#include <cstring>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define BUF_SIZE 4096
#define PADDING 200
#define ARRAY_SIZE 112 * 112 * 3

const char *s = "Hello World!";
TEST_CASE("SEAL/UNSEAL", "[seal]") {
  char buf[BUF_SIZE] = {0};
  memcpy(buf, s, strlen(s));
  int data_len = strlen(buf) + 1;
  int sealed_data_len = seal_data(buf, sizeof(buf), data_len);
  printf("SEALED_DATA_LEN: %d\n", sealed_data_len);
  REQUIRE(sealed_data_len > data_len);
  int unsealed_data_len = unseal_data(buf, sizeof(buf));
  REQUIRE(unsealed_data_len == data_len);
  // unseal(seal(data)) == data
  printf("DECRYPTEDTEXT: %s", buf);
  REQUIRE(strcmp(buf, "Hello World!") == 0);
}

TEST_CASE("SEAL/UNSEAL float array", "[seal]") {
  float arr[ARRAY_SIZE + PADDING];
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    arr[i] = i * 0.1f;
  }
  int sealed_data_len =
      seal_data((char *)arr, sizeof(arr), ARRAY_SIZE * sizeof(float));
  REQUIRE(sealed_data_len > ARRAY_SIZE);
  REQUIRE(sealed_data_len < sizeof(arr));
  int unsealed_data_len = unseal_data((char *)arr, sizeof(arr));
  REQUIRE(unsealed_data_len == ARRAY_SIZE * sizeof(float));
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    REQUIRE(arr[i] == i * 0.1f);
  }
}

TEST_CASE("GET_SEALED_DATA_FROM_ENCLAVE", "[seal]") {
  char ciphertext[200] = {0};
  int ret;
  ret = get_sealed_data(ciphertext, sizeof(ciphertext));
  REQUIRE(ret > 0);

  printf("CIPHERTEXT: ");
  for (int i = 0; i < sizeof(ciphertext); i++) {
    putchar(ciphertext[i]);
  }
  putchar('\n');

  ret = test_sealed_data(ciphertext, sizeof(ciphertext));
  REQUIRE(ret == 0);
}
