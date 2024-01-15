#include "pch.h"
#include "clang-c/CXString.h"
#include "clang-c/Index.h"

#include "parser.h"

#define PATTERN(name)                                                          \
  { std::regex("(\\$\\{" #name "\\})"), &SourceContext::name }

std::vector<std::pair<std::regex, decltype(&SourceContext::src)>> replaces{
    PATTERN(src),
    PATTERN(src_content),
    PATTERN(ret),
    PATTERN(params),
    PATTERN(comma_params),
    PATTERN(func_name),
    PATTERN(comma_param_names),
    PATTERN(root_cmake),
    PATTERN(host_secure_cmake),
    PATTERN(project),
    PATTERN(edl_params),
    PATTERN(src_path)};

std::string g_filepath;

template <bool WithType, bool IsEDL, bool WithCommaAhead>
std::string get_params_str(const std::vector<Param> &params) {
  std::stringstream ss;
  bool flag = false;

  // if without type, the params will be used after other params
  for (const auto &[type, name, array_size] : params) {
    if constexpr (!WithCommaAhead) {
      if (flag) {
        ss << ", ";
      }
    } else {
      ss << ", ";
    }
    if constexpr (IsEDL) {
      if (array_size >= 0) {
        ss << "[in, out, size=" << array_size << "] ";
      }
    }
    if constexpr (WithType) {
      ss << type << " " << name;
    } else {
      ss << name;
    }
    if constexpr (!WithCommaAhead) {
      flag = true;
    }
  }
  return ss.str();
}

std::string get_params(const std::vector<Param> &params) {
  return get_params_str<true, false, false>(params);
}

std::string get_comma_params(const std::vector<Param> &params) {
  return get_params_str<true, false, true>(params);
}

std::string get_comma_param_names(const std::vector<Param> &params) {
  return get_params_str<false, false, true>(params);
}

std::string get_edl_params(const std::vector<Param> &params) {
  return get_params_str<true, true, false>(params);
}

std::string getCursorSpelling(const CXCursor &cursor) {
  CXString cxStr = clang_getCursorSpelling(cursor);
  std::string str = clang_getCString(cxStr);
  clang_disposeString(cxStr);
  return str;
}

std::string getTypeSpelling(const CXType &type) {
  CXString cxStr = clang_getTypeSpelling(type);
  std::string str = clang_getCString(cxStr);
  clang_disposeString(cxStr);
  return str;
}

std::string getCursorType(const CXCursor &cursor) {
  CXType type = clang_getCursorType(cursor);
  return getTypeSpelling(type);
}

std::string getFunctionReturnType(CXCursor cursor) {
  CXType returnType = clang_getResultType(clang_getCursorType(cursor));
  return getTypeSpelling(returnType);
}

std::vector<Param> getFunctionParameters(CXCursor cursor) {
  std::vector<Param> parameters;
  int numArgs = clang_Cursor_getNumArguments(cursor);
  for (int i = 0; i < numArgs; ++i) {
    CXCursor arg = clang_Cursor_getArgument(cursor, i);

    const auto type = clang_getCursorType(arg);
    ASSERT(type.kind != CXType_Pointer,
           "In your secure function, please use constant array (e.g. char "
           "arr[32]) instead of pointer "
           "(e.g. char *arr or char arr[])");

    Param param = {.type = getCursorType(arg),
                   .name = getCursorSpelling(arg),
                   .array_size = -1};
    if (type.kind == CXType_ConstantArray) {
      param.array_size = clang_getArraySize(type);
      CXType pointee_type = clang_getArrayElementType(type);
      param.type = getTypeSpelling(pointee_type) + "*";
    }
    parameters.push_back(std::move(param));
  }
  return parameters;
}

std::string readSourceCode(const std::string &filePath) {
  std::ifstream file(filePath);
  std::stringstream buffer;

  if (file) {
    // 读取文件内容到 buffer
    buffer << file.rdbuf();
    file.close();
    // 将 buffer 转换为 string
    return buffer.str();
  } else {
    // 文件打开失败的处理
    std::cerr << "Unable to open file: " << filePath << std::endl;
    return "";
  }
}

std::string getFunctionBody(CXCursor cursor) {
  // 确保传入的游标是函数声明
  if (clang_getCursorKind(cursor) != CXCursor_FunctionDecl) {
    return ""; // 或者抛出异常
  }

  CXCursor bodyCursor = clang_getNullCursor(); // 初始化为null游标
  clang_visitChildren(
      cursor,
      [](CXCursor c, CXCursor parent, CXClientData clientData) {
        if (clang_getCursorKind(c) == CXCursor_CompoundStmt) {
          // 找到函数体
          *reinterpret_cast<CXCursor *>(clientData) = c;
          return CXChildVisit_Break; // 停止遍历
        }
        return CXChildVisit_Continue; // 继续遍历
      },
      &bodyCursor);
  // 获取函数体的范围
  if (bodyCursor.kind == CXCursor_InvalidCode) {
    return "";
  }
  CXSourceRange range = clang_getCursorExtent(bodyCursor);
  CXSourceLocation startLoc = clang_getRangeStart(range);
  CXSourceLocation endLoc = clang_getRangeEnd(range);

  // 获取源代码的位置信息
  unsigned int startOffset, endOffset;
  clang_getSpellingLocation(startLoc, nullptr, nullptr, nullptr, &startOffset);
  clang_getSpellingLocation(endLoc, nullptr, nullptr, nullptr, &endOffset);

  std::string sourceCode = readSourceCode(g_filepath); // 需要实现这个函数

  // 提取函数体
  std::string functionBody =
      sourceCode.substr(startOffset, endOffset - startOffset);

  return functionBody;
}

template <WorldType world_type_visited>
CXChildVisitResult
entry_func_def_collect_visitor(const CXCursor &cursor, const CXCursor &parent,
                               const CXClientData &clientData) {
  if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0) {
    return CXChildVisit_Continue;
  }

  auto kind = clang_getCursorKind(cursor);
  if (kind == CXCursor_FunctionDecl) {
    auto func_name = getCursorSpelling(cursor);

    bool is_def_valid;

    // the def must be called in another world to be an entry
    if constexpr (world_type_visited == WorldType::SECURE_WORLD) {
      is_def_valid = func_calls_in_insecure_world.count(func_name) != 0;
    } else {
      is_def_valid = func_calls_in_secure_world.count(func_name) != 0;
    }

    // insecure world can't call a static secure func in secure world!
    // so we can simply ignore static functions
    if (CX_SC_Static != clang_Cursor_getStorageClass(cursor) && is_def_valid) {
      FunctionInfo funcInfo;
      funcInfo.name = std::move(func_name);
      funcInfo.returnType = getFunctionReturnType(cursor);
      funcInfo.parameters = getFunctionParameters(cursor);
      funcInfo.body = getFunctionBody(cursor);

      // if body is not empty, then it is a definition
      if (!funcInfo.body.empty()) {
        func_list_each_file.push_back(std::move(funcInfo));
      }
    }
  }
  return CXChildVisit_Recurse;
}

CXChildVisitResult
insecure_world_entry_func_def_collect_visitor(CXCursor cursor, CXCursor parent,
                                              CXClientData clientData) {
  return entry_func_def_collect_visitor<WorldType::INSECURE_WORLD>(
      cursor, parent, clientData);
}

CXChildVisitResult
secure_world_entry_func_def_collect_visitor(CXCursor cursor, CXCursor parent,
                                            CXClientData clientData) {
  return entry_func_def_collect_visitor<WorldType::SECURE_WORLD>(cursor, parent,
                                                                 clientData);
}

CXChildVisitResult func_call_collect_visitor(CXCursor cursor, CXCursor parent,
                                             CXClientData clientData) {
  auto kind = clang_getCursorKind(cursor);
  if (kind == CXCursor_FunctionDecl) {
    func_calls_each_file.insert(getCursorSpelling(cursor));
  }
  if (kind == CXCursor_CallExpr) {
    cursor = clang_getCursorReferenced(cursor);
    if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {
      func_calls_each_file.insert(getCursorSpelling(cursor));
    }
  }
  return CXChildVisit_Recurse;
}

void parse_file(const char *path, VISITOR visitor, void *client_data) {
  CXIndex index = clang_createIndex(0, 0);
  g_filepath = path;
  const char *args[] = {"-E"};
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, path, args, sizeof(args) / sizeof(*args), nullptr, 0,
      CXTranslationUnit_None);

  if (unit == nullptr) {
    std::cerr << "Unable to parse translation unit: " << path << std::endl;
    return;
  }

  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(cursor, visitor, client_data);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

std::string parse_template(std::string templ, const SourceContext &ctx) {
  std::string res = std::move(templ);
  for (const auto &[pat, repl] : replaces) {
    res = std::regex_replace(res, pat, ctx.*repl);
  }
  return res;
}

std::string get_filepath(std::ifstream &ifs, const SourceContext &ctx) {
  assert(ifs);
  std::string label, filepath;
  ifs >> label;
  assert(label == "path:");

  ifs >> filepath;
  return parse_template(filepath, ctx);
}

std::string get_content(std::ifstream &ifs, const SourceContext &ctx) {
  assert(ifs);
  std::string line;
  std::stringstream ss;
  bool multi_template = false;
  bool global = false;
  bool insecure = false;
  std::vector<std::string> lines;
  while (std::getline(ifs, line)) {
    if (line.find("**begin**") != std::string::npos) {
      multi_template = true;
      continue;
    } else if (line.find("**gbegin**") != std::string::npos) {
      multi_template = true;
      global = true;
      continue;
    } else if (line.find("**igbegin**") != std::string::npos) {
      multi_template = true;
      global = true;
      insecure = true;
      continue;
    } else if (line.find("**end**") != std::string::npos) {

      SourceContext each_ctx = ctx;

      const auto &list = global ? (insecure ? insecure_entry_func_list
                                            : secure_entry_func_list)
                                : func_list_each_file;

      for (const auto &func : list) {
        each_ctx.func_name = func.name;
        each_ctx.params = get_params(func.parameters);
        each_ctx.comma_params = get_comma_params(func.parameters);
        each_ctx.comma_param_names = get_comma_param_names(func.parameters);
        each_ctx.edl_params = get_edl_params(func.parameters);
        each_ctx.ret = func.returnType;

        for (const auto &line : lines) {
          ss << parse_template(line, each_ctx) << std::endl;
        }
      }

      lines.clear();

      multi_template = false;
      global = false;
      insecure = false;
      continue;
    }
    if (multi_template) {
      lines.push_back(std::move(line));
    } else {
      ss << parse_template(std::move(line), ctx) << std::endl;
    }
  }
  return ss.str();
}

std::string read_file_content(const std::string &filename) {
  std::ifstream ifs(filename);
  std::stringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

void process_template(std::ifstream &ifs, const SourceContext &ctx) {
  const auto filepath = get_filepath(ifs, ctx);
  const auto content = get_content(ifs, ctx);

  const auto path = std::filesystem::path("./generated") / filepath;
  std::cout << "GENERATED:" << path << std::endl;
  std::filesystem::create_directories(path.parent_path());
  std::ofstream ofs(path);
  ofs << content;
}
