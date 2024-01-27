typedef char in_char;
typedef char out_char;

/*
 * @Param buf: contains data to be sealed, and will be used to store sealed data
 * @Param buf_len: capacity of buf
 * @Param data_len: length of data to be sealed
 * @Return: sealed data len on success, -1 on failure
 */
int seal_data(char *buf, int buf_len, int data_len);

/*
 * @Param buf: contains data to be unsealed, and will be used to store unsealed
 * data
 * @Param buf_len: capacity of buf
 * @Return: unseal_data on success, -1 on failure
 */
int unseal_data(char *buf, int buf_len);

int get_sealed_data(out_char *buf, int buf_len);
int test_sealed_data(in_char *buf, int buf_len);
