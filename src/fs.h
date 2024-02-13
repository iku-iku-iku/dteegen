#pragma once
#include "pch.h"

inline bool is_source_file(const std::filesystem::path &p) {
  return p.extension() == ".c" || p.extension() == ".cpp" ||
         p.extension() == ".cc";
}

template <typename VISITOR>
void for_each_in_dir(const std::filesystem::path &path, VISITOR visitor) {
  ASSERT(std::filesystem::is_directory(path), "%s not a directory", path.string().c_str());

  for (const auto &f : std::filesystem::directory_iterator(path)) {
    visitor(f);
  }
}

template <typename VISITOR>
void for_each_file_in_dir(const std::filesystem::path &path, VISITOR visitor) {
  ASSERT(std::filesystem::is_directory(path), "%s not a directory", path.string().c_str());

  for (const auto &f : std::filesystem::directory_iterator(path)) {
    if (f.is_directory()) {
      continue;
    }
    visitor(f);
  }
}

template <typename VISITOR>
void for_each_file_in_path_recursive(
    const std::filesystem::path &path, VISITOR visitor,
    const std::unordered_set<std::string> &skip_dirs = {}) {
  /* ASSERT(std::filesystem::is_directory(path), "not a directory"); */

  for_each_in_dir(path, [&](const auto &entry) {
    if (entry.is_directory()) {
      if (skip_dirs.find(entry.path().filename()) == skip_dirs.end()) {
        for_each_file_in_path_recursive(entry.path(), visitor, skip_dirs);
      }
      return;
    }
    /* std::cout << entry.path() << std::endl; */
    visitor(entry);
  });

  /* for (const auto &f : std::filesystem::recursive_directory_iterator(path)) {
   */
  /*   if (f.is_directory()) { */
  /*     continue; */
  /*   } */
  /*   visitor(f); */
  /* } */
}
