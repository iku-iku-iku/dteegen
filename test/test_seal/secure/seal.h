typedef char in_char;
typedef char out_char;

/*
 * @Param buf: contains data to be sealed, and will be used to store sealed data
 * @Param buf_len: capacity of buf
 * @Return: zero on success, non-zero on failure
 */
int seal_data(char *buf, int buf_len);

/*
 * @Param buf: contains data to be unsealed, and will be used to store unsealed
 * data
 * @Param buf_len: capacity of buf
 * @Return: zero on success, non-zero on failure
 */
int unseal_data(char *buf, int buf_len);
/* int seal_data(in_char *plaintext, int plaintext_len, out_char *ciphertext, */
/*               int ciphertext_len); */
/* int unseal_data(out_char *plaintext, int plaintext_len, in_char *ciphertext,
 */
/*                 int ciphertext_len); */

int get_sealed_data(out_char *ciphertext, int ciphertext_len);
int test_sealed_data(in_char *ciphertext, int ciphertext_len);
