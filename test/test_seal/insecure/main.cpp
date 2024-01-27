#include "../secure/seal.h"
#include <cstring>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define BUF_SIZE 4096
#define ARRAY_SIZE 112 * 112 * 3

char plaintext[] = "Hello World!";
TEST_CASE("SEAL/UNSEAL", "[seal]") {
  char ciphertext[BUF_SIZE] = {0};
  char decryptedtext[BUF_SIZE] = {0};
  int ret;
  ret = seal_data(plaintext, strlen(plaintext) + 1, ciphertext,
                  sizeof(ciphertext));
  REQUIRE(ret == 0);
  ret = unseal_data(decryptedtext, sizeof(decryptedtext), ciphertext,
                    sizeof(ciphertext));
  REQUIRE(ret == 0);
  // unseal(seal(data)) == data

  printf("DECRYPTEDTEXT: %s", decryptedtext);
  REQUIRE(strcmp(plaintext, decryptedtext) == 0);
}

TEST_CASE("SEAL/UNSEAL float array", "[seal]") {
  float arr[ARRAY_SIZE];
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    arr[i] = i * 0.1f;
  }
  char ciphertext[ARRAY_SIZE * 2 * sizeof(float)];
  char decryptedtext[ARRAY_SIZE * 2 * sizeof(float)];
  int ret;
  ret = seal_data((char *)arr, sizeof(arr), ciphertext, sizeof(ciphertext));
  REQUIRE(ret == 0);
  ret = unseal_data(decryptedtext, sizeof(decryptedtext), ciphertext,
                    sizeof(ciphertext));
  REQUIRE(ret == 0);
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    REQUIRE(arr[i] == ((float *)decryptedtext)[i]);
  }
}

TEST_CASE("GET_SEALED_DATA", "[seal]") {
  char ciphertext[200] = {0};
  int ret;
  ret = get_sealed_data(ciphertext, sizeof(ciphertext));
  REQUIRE(ret == 0);

  printf("CIPHERTEXT: ");
  for (int i = 0; i < sizeof(ciphertext); i++) {
    putchar(ciphertext[i]);
  }
  putchar('\n');

  ret = test_sealed_data(ciphertext, sizeof(ciphertext));
  REQUIRE(ret == 0);
}
