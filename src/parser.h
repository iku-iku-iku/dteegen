#pragma once

#include "template.h"
#include <clang-c/Index.h>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

struct Param {
  std::string type;
  std::string name;
  int array_size;
  bool is_in;
  bool is_out;
  bool is_ptr;
  bool is_array;
};

struct FileContext {
  std::string file_path;
};

struct FunctionInfo {
  std::string name;
  std::string returnType;
  std::vector<Param> parameters;
  std::string body;
};

using VISITOR = CXChildVisitResult (*)(CXCursor cursor, CXCursor parent,
                                       CXClientData client_data);

std::string read_file_content(const std::string &filename);

void parse_file(const FileContext &file_ctx, VISITOR visitor);

CXChildVisitResult func_call_collect_visitor(CXCursor cursor, CXCursor parent,
                                             CXClientData clientData);

// a entry func def in insecure world is called in secure world
CXChildVisitResult
insecure_world_entry_func_def_collect_visitor(CXCursor cursor, CXCursor parent,
                                              CXClientData clientData);

// a entry func def in secure world is called in insecure world
CXChildVisitResult
secure_world_entry_func_def_collect_visitor(CXCursor cursor, CXCursor parent,
                                            CXClientData clientData);

inline std::mutex for_each_file_mutex;

inline thread_local std::vector<FunctionInfo> tls_func_list_each_file;
inline thread_local std::vector<FunctionInfo> tls_secure_entry_func_list;
inline thread_local std::vector<FunctionInfo> tls_insecure_entry_func_list;
inline std::vector<FunctionInfo> g_secure_entry_func_list;
inline std::vector<FunctionInfo> g_insecure_entry_func_list;
using FuncName = std::string;
inline thread_local std::unordered_set<FuncName> tls_func_calls_each_file;
inline thread_local std::unordered_set<FuncName>
    tls_func_calls_in_insecure_world;
inline thread_local std::unordered_set<FuncName> tls_func_calls_in_secure_world;
inline std::unordered_set<FuncName> g_func_calls_in_insecure_world;
inline std::unordered_set<FuncName> g_func_calls_in_secure_world;
