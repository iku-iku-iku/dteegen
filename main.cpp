#include "CLI11.hpp"
#include "clang-c/CXString.h"
#include <cassert>
#include <clang-c/Index.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#define ASSERT(x, msg)                                                         \
  if (!(x)) {                                                                  \
    std::cerr << msg << std::endl;                                             \
    exit(-1);                                                                  \
  }

struct Param {
  std::string type;
  std::string name;
  int array_size;
};

template <bool WithParam, bool IsEDL>
std::string get_params_str(const std::vector<Param> &params) {
  std::stringstream ss;
  bool flag = false;
  for (const auto &[type, name, array_size] : params) {
    if (flag) {
      ss << ", ";
    }
    if constexpr (IsEDL) {
      if (array_size >= 0) {
        ss << "[in, out, size=" << array_size << "] ";
      }
    }
    if constexpr (WithParam) {
      ss << type << " " << name;
    } else {
      ss << name;
    }
    flag = true;
  }
  return ss.str();
}

std::string get_params(const std::vector<Param> &params) {
  return get_params_str<true, false>(params);
}

std::string get_param_names(const std::vector<Param> &params) {
  return get_params_str<false, false>(params);
}

std::string get_edl_params(const std::vector<Param> &params) {
  return get_params_str<true, true>(params);
}

struct FunctionInfo {
  std::string name;
  std::string returnType;
  std::vector<Param> parameters;
  std::string body;
};

std::vector<FunctionInfo> func_list;

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

std::string g_filepath;

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
  CXSourceRange range = clang_getCursorExtent(bodyCursor);
  CXSourceLocation startLoc = clang_getRangeStart(range);
  CXSourceLocation endLoc = clang_getRangeEnd(range);

  // 获取源代码的位置信息
  unsigned int startOffset, endOffset;
  clang_getSpellingLocation(startLoc, nullptr, nullptr, nullptr, &startOffset);
  clang_getSpellingLocation(endLoc, nullptr, nullptr, nullptr, &endOffset);

  // 读取源文件内容（这里假设你有途径获取源文件内容）
  std::string sourceCode = readSourceCode(g_filepath); // 需要实现这个函数

  // 提取函数体
  std::string functionBody =
      sourceCode.substr(startOffset, endOffset - startOffset);

  return functionBody;
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor parent,
                           CXClientData clientData) {
  auto kind = clang_getCursorKind(cursor);
  if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0) {
    return CXChildVisit_Continue;
  }
  if (kind == CXCursor_FunctionDecl) {
    FunctionInfo funcInfo;
    funcInfo.name = getCursorSpelling(cursor);
    funcInfo.returnType = getFunctionReturnType(cursor);
    funcInfo.parameters = getFunctionParameters(cursor);
    funcInfo.body = getFunctionBody(cursor);
    func_list.push_back(std::move(funcInfo));
  }
  return CXChildVisit_Recurse;
}

void parse_file(const char *path) {
  CXIndex index = clang_createIndex(0, 0);
  g_filepath = path;
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, path, nullptr, 0, nullptr, 0, CXTranslationUnit_None);

  if (unit == nullptr) {
    std::cerr << "Unable to parse translation unit. Quitting.\n";
    return;
  }

  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(cursor, visitor, nullptr);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

#define PATTERN(name)                                                          \
  { std::regex("(\\$\\{" #name "\\})"), &SourceContext::name }

struct SourceContext {
  std::string project;
  std::string src;
  std::string src_content;
  std::string ret;
  std::string params;
  std::string param_names;
  std::string func_name;
  std::string interfaces;
};

std::vector<std::pair<std::regex, decltype(&SourceContext::src)>> replaces{
    PATTERN(src),        PATTERN(src_content), PATTERN(ret),
    PATTERN(params),     PATTERN(func_name),   PATTERN(param_names),
    PATTERN(interfaces), PATTERN(project)};

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
  while (std::getline(ifs, line)) {
    ss << parse_template(line, ctx) << std::endl;
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

void generate_secgear(const std::filesystem::path project_root) {

  std::filesystem::path generated_path("generated");

  // remove generated first
  if (std::filesystem::exists(generated_path)) {
    std::filesystem::remove_all(generated_path);
  }

  SourceContext ctx;
  ctx.project = project_root.filename();

  for (const auto &secure_func_file :
       std::filesystem::directory_iterator(project_root / "enclave")) {
    const auto secure_func_filepath = secure_func_file.path();

    parse_file(secure_func_filepath.c_str());

    ctx.src = secure_func_filepath.stem().string();
    ctx.src_content = read_file_content(secure_func_filepath);
    // ctx.template_filename = e.path().filename().string();
    ctx.ret = func_list[func_list.size() - 1].returnType;
    ctx.func_name = func_list[func_list.size() - 1].name;
    ctx.params = get_params(func_list[0].parameters);
    ctx.param_names = get_param_names(func_list[0].parameters);

    for (const auto &e :
         std::filesystem::directory_iterator("template/secure_func_template")) {
      std::ifstream ifs(e.path());

      process_template(ifs, ctx);
    }
  }

  std::stringstream interfaces;
  for (const auto &func : func_list) {
    interfaces << "public " << func.returnType << " " << func.name << "_impl("
               << get_edl_params(func.parameters) << ");\n";
  }
  ctx.interfaces = interfaces.str();

  for (const auto &e :
       std::filesystem::directory_iterator("template/project_template")) {
    std::ifstream ifs(e.path());

    process_template(ifs, ctx);
  }
  const auto generated_host = std::filesystem::path("generated/host");

  // copy host
  for (const auto &f :
       std::filesystem::directory_iterator(project_root / "host")) {
    std::filesystem::copy_options op;
    op |= std::filesystem::copy_options::overwrite_existing;
    std::filesystem::copy_file(f.path(), generated_host / f.path().filename(),
                               op);
  }
}

// 主函数
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " project\n";
    return 1;
  }
  generate_secgear(argv[1]);
  return 0;
}
