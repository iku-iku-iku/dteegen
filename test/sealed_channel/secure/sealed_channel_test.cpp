#include "sealed_channel_test.h"
#include "../insecure/sender.h"
#include <cstring>

int send() {
  char secret[SECRET_LEN];
  generate_random_secret<SECRET_LEN>(secret);
  char buf[BUF_LEN];
  memcpy(buf, secret, SECRET_LEN);

  int res;
  res = seal_data_inplace(buf, BUF_LEN, SECRET_LEN);
  if (-1 == res) {
    return res;
  }

  res = sender(buf, BUF_LEN);
  if (res != 0) {
    return -1;
  }

  eapp_print("SEND SUCCESS: ");
  print_random_secret<16>(secret);
  return 0;
}

int receive(in_char buf[BUF_LEN]) {
  char secret[SECRET_LEN];
  int res;
  if (-1 == (res = unseal_data_inplace(buf, BUF_LEN))) {
    return res;
  }
  memcpy(secret, buf, SECRET_LEN);

  eapp_print("RECEIVE SUCCESS: ");
  print_random_secret<16>(secret);
  return 0;
}
