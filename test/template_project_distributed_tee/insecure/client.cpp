#include "../secure/add.h"
#include "distributed_tee.h"

int main() {
  auto ctx = init_distributed_tee_context({.side = SIDE::Client,
                                           .mode = MODE::Transparent,
                                           .name = "template_client",
                                           .version = "1.0"});
  // if (exist_local_tee()) {
  //   printf("detected tee\n");
  // } else {
  //   printf("no tee\n");
  // }

  int res;
  int a = 1, b = 2;
  res = mul_x_and_y(a, b);
  printf("mul(%d, %d) == %d\n", a, b, res);
  res = add_x_and_y(a, b);
  printf("add(%d, %d) == %d\n", a, b, res);
  // this will be execute secure func locally if there is tee locally, otherwise
  // it's the same as call_remote_secure_function
  /* res = call_distributed_secure_function(ctx, mul, 1, 2); */
  /* printf("mul(%d, %d) == %d\n", a, b, res); */
  /* res = call_remote_secure_function(ctx, mul, 1, 2); */
  /* printf("mul(%d, %d) == %d\n", a, b, res); */
  /* res = call_remote_secure_function(ctx, add, 1, 2); */
  /* printf("add(%d, %d) == %d\n", a, b, res); */
  destroy_distributed_tee_context(ctx);
}
