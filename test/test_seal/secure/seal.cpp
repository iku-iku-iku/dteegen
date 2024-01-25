#include "seal.h"
#include <TEE-Capability/common.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdlib.h>
const char *secret = "hello wolrd";

typedef struct _enclave_sealed_data_t {
  uint32_t data_body_len;
  uint8_t reserved[16];
  uint8_t data_body[];
} cc_enclave_sealed_data_t;
char additional_text[] = "add mac text";

uint32_t cc_enclave_get_sealed_data_size(const uint32_t add_len,
                                         const uint32_t seal_data_len) {
  return sizeof(cc_enclave_sealed_data_t) + seal_data_len + add_len;
}

uint32_t cc_enclave_seal_data(uint8_t *seal_data, uint32_t seal_data_len,
                              cc_enclave_sealed_data_t *sealed_data,
                              uint32_t sealed_data_len,
                              uint8_t *additional_text,
                              uint32_t additional_text_len) {
  memcpy(sealed_data->reserved, &seal_data_len, sizeof(uint32_t));
  memcpy(sealed_data->reserved + sizeof(uint32_t), &additional_text_len,
         sizeof(uint32_t));

  uint8_t *p = sealed_data->data_body;
  for (int i = 0; i < seal_data_len; i++) {
    *p++ = 0xff ^ seal_data[i];
  }
  for (int i = 0; i < additional_text_len; i++) {
    *p++ = 0xfe ^ additional_text[i];
  }
  return 0;
}

uint32_t cc_enclave_unseal_data(cc_enclave_sealed_data_t *sealed_data,
                                uint8_t *decrypted_data,
                                uint32_t *decrypted_data_len,
                                uint8_t *additional_text,
                                uint32_t *additional_text_len) {
  uint8_t *p = sealed_data->data_body;
  for (int i = 0; i < *decrypted_data_len; i++) {
    decrypted_data[i] = 0xff ^ *p++;
  }
  for (int i = 0; i < *additional_text_len; i++) {
    additional_text[i] = 0xfe ^ *p++;
  }
  return 0;
}

uint32_t
cc_enclave_get_add_text_size(const cc_enclave_sealed_data_t *sealed_data) {

  uint32_t add_text_len;
  memcpy(&add_text_len, sealed_data->reserved + sizeof(uint32_t),
         sizeof(uint32_t));
  return add_text_len;
}

uint32_t cc_enclave_get_encrypted_text_size(
    const cc_enclave_sealed_data_t *sealed_data) {
  uint32_t encrypted_text_len;
  memcpy(&encrypted_text_len, sealed_data->reserved, sizeof(uint32_t));
  return encrypted_text_len;
}

int seal_data(char plaintext[64], char ciphertext[64]) {

  uint32_t ret;
  long data_len = strlen(plaintext);
  long add_len = strlen((const char *)additional_text);

  uint32_t sealed_data_len = cc_enclave_get_sealed_data_size(data_len, add_len);

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

int unseal_data(char plaintext[64], char ciphertext[64]) {
  char *sealed_data = ciphertext;
  uint32_t encrypt_add_len = cc_enclave_get_add_text_size(
      (const cc_enclave_sealed_data_t *)sealed_data);
  uint32_t encrypt_data_len = cc_enclave_get_encrypted_text_size(
      (const cc_enclave_sealed_data_t *)sealed_data);

  char *decrypted_seal_data = (char *)malloc(encrypt_data_len);
  char *demac_data = (char *)malloc(encrypt_add_len);

  cc_enclave_unseal_data((cc_enclave_sealed_data_t *)sealed_data,
                         (uint8_t *)decrypted_seal_data, &encrypt_data_len,
                         (uint8_t *)demac_data, &encrypt_add_len);

  if (strcmp(demac_data, additional_text) != 0) {
    return -1;
  }

  strncpy(plaintext, decrypted_seal_data, encrypt_data_len);
  return 0;
}
