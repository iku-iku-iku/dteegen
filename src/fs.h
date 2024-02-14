#pragma once
#include "pch.h"
#include "thread_pool.h"

inline std::string read_file(const std::string &file_path) {
  std::ifstream file(file_path);
  std::stringstream buffer;

  if (file) {
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
  } else {
    std::cerr << "Unable to open file: " << file_path << std::endl;
    return "";
  }
}

inline bool is_source_file(const std::filesystem::path &p) {
  return p.extension() == ".c" || p.extension() == ".cpp" ||
         p.extension() == ".cc";
}

template <typename VISITOR>
void for_each_in_dir(const std::filesystem::path &path, VISITOR visitor) {
  ASSERT(std::filesystem::is_directory(path), "%s not a directory",
         path.string().c_str());

  for (const auto &f : std::filesystem::directory_iterator(path)) {
    visitor(f);
  }
}

template <typename VISITOR>
void for_each_file_in_dir(const std::filesystem::path &path, VISITOR visitor) {
  ASSERT(std::filesystem::is_directory(path), "%s not a directory",
         path.string().c_str());

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
}

template <typename VISITOR>
void for_each_file_in_path_recursive_parallel(
    const std::filesystem::path &path, VISITOR &&visitor,
    const std::unordered_set<std::string> &skip_dirs, ThreadPool &pool) {
  /* return for_each_file_in_path_recursive(path, visitor, skip_dirs); */
  /* ASSERT(std::filesystem::is_directory(path), "not a directory"); */

  for_each_in_dir(path, [&](const auto &entry) {
    if (entry.is_directory()) {
      if (skip_dirs.find(entry.path().filename()) == skip_dirs.end()) {
        for_each_file_in_path_recursive_parallel(entry.path(), visitor,
                                                 skip_dirs, pool);
      }
      return;
    }
    /* std::cout << entry.path() << std::endl; */
    pool.enqueue([&visitor, entry] { visitor(entry); });
  });
}
