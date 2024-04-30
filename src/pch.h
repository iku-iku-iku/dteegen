#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


enum class WorldType : uint8_t { SECURE_WORLD, INSECURE_WORLD };

#define DTEE_LOG(...) printf(__VA_ARGS__)

#define ASSERT(x, msg, ...)                                                    \
  if (!(x)) {                                                                  \
    fprintf(stderr, "Assertion failed: %s, " msg "\n", #x, ##__VA_ARGS__);     \
    exit(-1);                                                                  \
  }
