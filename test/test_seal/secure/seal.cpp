#include "seal.h"
#include <TEE-Capability/common.h>
#include <cstdint>
#include <cstring>
#include <iostream>

char additional_text[] = "add mac text";

int seal_data(char *buf, int buf_len, int data_len) {
  return seal_data_inplace(buf, buf_len, data_len);
}
/* int seal_data(char *buf, int buf_len, int data_len) { */
/*   uint32_t ret; */
/*   char *plaintext = buf; */
/*   /* long data_len = plaintext_len; */
/*   long add_len = strlen((const char *)additional_text) + 1; */
/**/
/*   uint32_t sealed_data_len = cc_enclave_get_sealed_data_size(add_len,
   data_len); */
/*   if (sealed_data_len > buf_len) { */
/*     eapp_print("sealed_data_len > buf_len (%d > %d)\n", sealed_data_len,
 */
/*                buf_len); */
/*     return -1; */
/*   } */
/**/
/*   if (sealed_data_len == UINT32_MAX) */
/*     return -1; */
/**/
/*   cc_enclave_sealed_data_t *sealed_data = */
/*       (cc_enclave_sealed_data_t *)malloc(sealed_data_len); */
/*   if (sealed_data == NULL) */
/*     return -1; */
/**/
/*   ret = cc_enclave_seal_data((uint8_t *)plaintext, data_len, sealed_data,
 */
/*                              sealed_data_len, (uint8_t *)additional_text,
 */
/*                              add_len); */
/*   memcpy(buf, (const char *)sealed_data, sealed_data_len); */
/*   return sealed_data_len; */
/* } */

int unseal_data(char *buf, int buf_len) {
  return unseal_data_inplace(buf, buf_len);
}
/* int unseal_data(char *buf, int buf_len) { */
/*   char *sealed_data = buf; */
/*   uint32_t add_len = cc_enclave_get_add_text_size( */
/*       (const cc_enclave_sealed_data_t *)sealed_data); */
/*   uint32_t data_len = cc_enclave_get_encrypted_text_size( */
/*       (const cc_enclave_sealed_data_t *)sealed_data); */
/**/
/*   char *decrypted_data = (char *)malloc(data_len); */
/*   char *demac_data = (char *)malloc(add_len); */
/**/
/*   cc_enclave_unseal_data((cc_enclave_sealed_data_t *)sealed_data, */
/*                          (uint8_t *)decrypted_data, &data_len, */
/*                          (uint8_t *)demac_data, &add_len); */
/**/
/*   if (strcmp(demac_data, additional_text) != 0) { */
/*     return -1; */
/*   } */
/**/
/*   memcpy(buf, decrypted_data, data_len); */
/*   return data_len; */
/* } */

static char secret[] = "secret";
int get_sealed_data(out_char *buf, int buf_len) {
  int data_len = strlen(secret) + 1;
  memcpy(buf, secret, data_len);
  return seal_data(buf, buf_len, data_len);
}
int test_sealed_data(in_char *buf, int buf_len) {
  int unsealed_data_len = unseal_data(buf, buf_len);
  if (strcmp(buf, secret)) {
    return -1;
  }
  return 0;
}
