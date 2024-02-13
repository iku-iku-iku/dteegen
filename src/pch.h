#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define SECURE "secure"
#define INSECURE "insecure"
#define ENCLAVE "enclave"
#define HOST "host"
#define SECURE_LIB "secure_lib"
#define ENCLAVE_LIB "enclave_lib"
#define SECURE_INCLUDE "secure_include"
#define ENCLAVE_INCLUDE "enclave_include"

enum class WorldType : uint8_t { SECURE_WORLD, INSECURE_WORLD };

#define ASSERT(x, msg, ...)                                                    \
  if (!(x)) {                                                                  \
    fprintf(stderr, "Assertion failed: %s, " msg "\n", #x, ##__VA_ARGS__);     \
    exit(-1);                                                                  \
  }
