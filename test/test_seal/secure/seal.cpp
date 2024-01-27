#include "seal.h"
#include <TEE-Capability/common.h>
#include <cstdint>
#include <cstring>
#include <iostream>

char additional_text[] = "add mac text";
/* int seal_data(in_char *plaintext, int plaintext_len, out_char *ciphertext, */
/*               int ciphertext_len) { */

int seal_data(char *buf, int buf_len) {

  uint32_t ret;
  /* long data_len = plaintext_len; */
  long data_len = buf_len;
  long add_len = strlen((const char *)additional_text);

  uint32_t sealed_data_len = cc_enclave_get_sealed_data_size(add_len, data_len);
  if (sealed_data_len > ciphertext_len) {
    eapp_print("sealed_data_len > ciphertext_len (%d > %d)\n", sealed_data_len,
               ciphertext_len);
    return -1;
  }

  eapp_print("SEALED_DATA_LEN: %d\n", sealed_data_len);

  if (sealed_data_len == UINT32_MAX)
    return -1;

  cc_enclave_sealed_data_t *sealed_data =
      (cc_enclave_sealed_data_t *)malloc(sealed_data_len);
  if (sealed_data == NULL)
    return -1;

  ret = cc_enclave_seal_data((uint8_t *)plaintext, data_len, sealed_data,
                             sealed_data_len, (uint8_t *)additional_text,
                             add_len);
  memcpy(ciphertext, (const char *)sealed_data, sealed_data_len);
  return ret;
}

int unseal_data(out_char *plaintext, int plaintext_len, in_char *ciphertext,
                int ciphertext_len) {
  char *sealed_data = ciphertext;
  uint32_t add_len = cc_enclave_get_add_text_size(
      (const cc_enclave_sealed_data_t *)sealed_data);
  uint32_t data_len = cc_enclave_get_encrypted_text_size(
      (const cc_enclave_sealed_data_t *)sealed_data);

  char *decrypted_data = (char *)malloc(data_len);
  char *demac_data = (char *)malloc(add_len);

  cc_enclave_unseal_data((cc_enclave_sealed_data_t *)sealed_data,
                         (uint8_t *)decrypted_data, &data_len,
                         (uint8_t *)demac_data, &add_len);

  if (strcmp(demac_data, additional_text) != 0) {
    return -1;
  }

  memcpy(plaintext, decrypted_data, data_len);
  return 0;
}

static char secret[] = "secret";
int get_sealed_data(out_char *ciphertext, int ciphertext_len) {
  return seal_data(secret, strlen(secret), ciphertext, ciphertext_len);
}
int test_sealed_data(in_char *ciphertext, int ciphertext_len) {
  char dec[4096];
  int ret;
  if ((ret = unseal_data(dec, sizeof(dec), ciphertext, ciphertext_len))) {
    return ret;
  }
  if (strcmp(dec, secret)) {
    return -1;
  }
  return 0;
}
