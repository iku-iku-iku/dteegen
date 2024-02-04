#include "../secure/sealed_channel_test.h"
#include <vector>
int receiver(std::vector<char> buf) {
  if (buf.size() > BUF_LEN) {
    return -1;
  }
  return receive(buf.data());
}
