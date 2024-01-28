#define WIDTH 112
#define HEIGHT WIDTH
#define IMG_SIZE 1 * WIDTH *HEIGHT * 3
#define EMB_LEN 128
#define EMBEDDING_SIZE 1 * EMB_LEN * sizeof(float) + 200
typedef char in_char;
typedef char out_char;
int embedding(in_char img[IMG_SIZE], out_char res[EMBEDDING_SIZE]);
