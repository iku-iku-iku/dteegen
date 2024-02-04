#include <TEE-Capability/common.h>
#include <ctime>
constexpr int BUF_LEN = 1024;
constexpr int SECRET_LEN = 16;

template <int SIZE> void generate_random_secret(char secret[SIZE]) {
  srand(time(0));
  for (int i = 0; i < SIZE; i++) {
    secret[i] = rand() % 256;
  }
}

template <int SIZE> void print_random_secret(char secret[SIZE]) {
  for (int i = 0; i < SIZE; i++) {
    eapp_print("%02x ", (unsigned char)secret[i]);
  }
  eapp_print("\n");
}
