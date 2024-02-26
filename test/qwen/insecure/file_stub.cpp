#include "file_stub.h"
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

namespace {

struct IO {
  static int Open(const char *filename) { return open(filename, O_RDONLY); }
  static bool Close(int fd) { return close(fd) == 0; }
  // Returns size in bytes or 0.
  static uint64_t FileSize(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd >= 0) {
      const off_t size = lseek(fd, 0, SEEK_END);
      if (size != static_cast<off_t>(-1)) {
        return static_cast<uint64_t>(size);
      }
    }

    return 0;
  }

  static int Read(int fd, uint64_t offset, uint64_t size, void *to) {
    uint8_t *bytes = reinterpret_cast<uint8_t *>(to);
    uint64_t pos = 0;
    for (;;) {
      // pread seems to be faster than lseek + read when parallelized.
      const auto bytes_read = pread(fd, bytes + pos, size - pos, offset + pos);
      if (bytes_read <= 0)
        break;
      pos += bytes_read;
      if (pos == size)
        break;
    }
    return pos; // success if managed to read desired size
  }

  static bool Write(const void *from, uint64_t size, uint64_t offset, int fd) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(from);
    uint64_t pos = 0;
    for (;;) {
      const auto bytes_written =
          pwrite(fd, bytes + pos, size - pos, offset + pos);
      if (bytes_written <= 0)
        break;
      pos += bytes_written;
      if (pos == size)
        break;
    }
    return pos == size; // success if managed to write desired size
  }
}; // IO
} // namespace

struct FileProvider {
  FileProvider(const char *filename) {
    ifs.open(filename, std::ios::binary);
    if (!ifs.is_open()) {
      std::cerr << "Error: could not open file " << filename << "\n";
    }
  }

  int read(char *buf, int buf_len) {
    ifs.read(buf, buf_len);
    return ifs.gcount();
  }

  std::ifstream ifs;
};

extern "C" int read_file(char *filename, int filename_len, int offset,
                         char *buf, int buf_len) {
  int fd = IO::Open(filename);

  int read_len = IO::Read(fd, offset, 1024, buf);
  IO::Close(fd);

  return read_len;
}

/* FileProvider model_file("model"); */
/* FileProvider tokenizer_file("tokenizer"); */
