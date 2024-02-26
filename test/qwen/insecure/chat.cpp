#include "../secure/llm.h"
#include "file_stub.h"
#define USED(x) ((void)(x))

int main() {
  USED(read_file);

  char user_input[] = "hello";
  llm_inference(user_input, sizeof(user_input));
}
