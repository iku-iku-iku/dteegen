#include "clang-c/CXString.h"
#include <clang-c/Index.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct FunctionInfo {
  std::string name;
  std::string returnType;
  std::vector<std::string> parameters;
  std::string body;
};

std::string getCursorSpelling(CXCursor cursor) {
  CXString cxStr = clang_getCursorSpelling(cursor);
  std::string str = clang_getCString(cxStr);
  clang_disposeString(cxStr);
  return str;
}

std::string getCursorType(CXCursor cursor) {
  CXType type = clang_getCursorType(cursor);
  CXString cxStr = clang_getTypeSpelling(type);
  std::string str = clang_getCString(cxStr);
  clang_disposeString(cxStr);
  return str;
}

std::string getFunctionReturnType(CXCursor cursor) {
  CXType returnType = clang_getResultType(clang_getCursorType(cursor));
  CXString returnSpelling = clang_getTypeSpelling(returnType);
  std::string str = clang_getCString(returnSpelling);
  clang_disposeString(returnSpelling);
  return str;
}

std::vector<std::string> getFunctionParameters(CXCursor cursor) {
  std::vector<std::string> parameters;
  int numArgs = clang_Cursor_getNumArguments(cursor);
  for (int i = 0; i < numArgs; ++i) {
    CXCursor arg = clang_Cursor_getArgument(cursor, i);
    parameters.push_back(getCursorType(arg) + " " + getCursorSpelling(arg));
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
  /* auto spell = clang_getCursorKindSpelling(clang_getCursorKind(cursor)); */
  /* std::cout << clang_getCString(spell) << std::endl; */
  /* clang_disposeString(spell); */
  /**/
  if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {
    FunctionInfo funcInfo;
    funcInfo.name = getCursorSpelling(cursor);
    funcInfo.returnType = getFunctionReturnType(cursor);
    funcInfo.parameters = getFunctionParameters(cursor);
    funcInfo.body = getFunctionBody(cursor);

    std::cout << "Function Name: " << funcInfo.name << std::endl;
    std::cout << "Return Type: " << funcInfo.returnType << std::endl;
    std::cout << "Parameters: ";
    for (const auto &param : funcInfo.parameters) {
      std::cout << param << ", ";
    }
    std::cout << "\nFunction Body: " << funcInfo.body << "\n\n";
  }
  return CXChildVisit_Recurse;
}

// 主函数
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>\n";
    return 1;
  }

  CXIndex index = clang_createIndex(0, 0);
  g_filepath = argv[1];
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, argv[1], nullptr, 0, nullptr, 0, CXTranslationUnit_None);

  if (unit == nullptr) {
    std::cerr << "Unable to parse translation unit. Quitting.\n";
    return 1;
  }

  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(cursor, visitor, nullptr);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
  return 0;
}
