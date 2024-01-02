#pragma once
#include "pch.h"

template <typename VISITOR>
void for_each_file_in_dir(const std::filesystem::path &path, VISITOR visitor) {
  ASSERT(std::filesystem::is_directory(path), "not a directory");

  for (const auto &f : std::filesystem::directory_iterator(path)) {
    if (f.is_directory()) {
      continue;
    }
    visitor(f);
  }
}

template <typename VISITOR>
void for_each_file_in_dir_recursive(const std::filesystem::path &path,
                                    VISITOR visitor) {
  ASSERT(std::filesystem::is_directory(path), "not a directory");

  for (const auto &f : std::filesystem::recursive_directory_iterator(path)) {
    if (f.is_directory()) {
      continue;
    }
    visitor(f);
  }
}
