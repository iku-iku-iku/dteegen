#include "file_stub.h"

#include <unistd.h>

#include <cstdio>

extern "C" size_t read_file(size_t f, size_t size, size_t n, out_char *buf,
                            int buf_len)
{
    printf("BEGIN READ: %lu %lu\n", size, n);
    auto res = fread(buf, size, n, reinterpret_cast<FILE *>(f));
    printf("END READ\n");
    return res;
}

extern "C" size_t close_file(size_t f)
{
    return fclose(reinterpret_cast<FILE *>(f));
}

extern "C" size_t open_file(char *filename, int filename_len)
{
    auto f = fopen(filename, "r");
    return (size_t)f;
}

extern "C" long int tell_file(size_t f)
{
    return ftell(reinterpret_cast<FILE *>(f));
}

extern "C" int seek_file(size_t f, long int offset, int whence)
{
    return fseek(reinterpret_cast<FILE *>(f), offset, whence);
}

/* FileProvider model_file("model"); */
/* FileProvider tokenizer_file("tokenizer"); */
