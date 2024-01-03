#include "../distribute_tee_framework.h"
#include "add.h"
#include "hello.h"
#include <stdio.h>

int main(int argc, char **argv) {
  char buf[32];
  /* char *buf = new char[32]; */
  get_string1(buf);
  printf("The result is %s\n", buf);

  printf("add(3, 4) = %d\n", add(3, 4));

  /* init_distributed_tee_framework(); */

  // if run as client
  // if (strcmp(argv[1], "client")) {
  //   call_remote_secure_function(get_string2, buf);
  //   printf("The result is %s\n", buf);
  // } else if (strcmp(argv[1], "server")) {
  //   publish_secure_function(get_string2);
  // }

  get_string3(buf);
  printf("The result is %s\n", buf);
}
