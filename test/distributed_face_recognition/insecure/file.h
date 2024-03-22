#pragma once
// #include "../secure/embedding.h"
#define MAX_EMB_CNT 10000
typedef char in_char;
typedef char out_char;
extern "C" int write_file(in_char* in_filename, int in_filename_len, in_char* in_content, int in_content_len);
extern "C" int get_emb_list(char out_list[sizeof(int) * MAX_EMB_CNT]);
extern "C" int read_file(char* in_filename, int in_filename_len, char* out_content, int out_content_len);

// int img_recorder(std::array<char, IMG_SIZE> arr, int id);

// int img_verifier(std::array<char, IMG_SIZE> arr);
