#include "parser.h"

#include "clang-c/CXString.h"
#include "clang-c/Index.h"
#include "fs.h"
#include "pch.h"

std::string getCursorSpelling(const CXCursor &cursor)
{
    CXString cxStr = clang_getCursorSpelling(cursor);
    std::string str = clang_getCString(cxStr);
    clang_disposeString(cxStr);
    return str;
}

std::string getTypeSpelling(const CXType &type)
{
    CXString cxStr = clang_getTypeSpelling(type);
    std::string str = clang_getCString(cxStr);
    clang_disposeString(cxStr);
    return str;
}

std::string getCursorType(const CXCursor &cursor)
{
    CXType type = clang_getCursorType(cursor);
    return getTypeSpelling(type);
}

std::string getFunctionReturnType(CXCursor cursor)
{
    CXType returnType = clang_getResultType(clang_getCursorType(cursor));
    return getTypeSpelling(returnType);
}

std::vector<Param> getFunctionParameters(CXCursor cursor)
{
    int numArgs = clang_Cursor_getNumArguments(cursor);
    std::vector<Param> parameters(numArgs);
    // check parameters validity
    static const std::unordered_set<std::string> valid_element_type = {
        "char", "in_char", "out_char"};

    std::string next_arg_name;
    for (int i = numArgs - 1; i >= 0; --i) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);

        const auto arg_name = getCursorSpelling(arg);

        const CXType type = clang_getCursorType(arg);
        if (type.kind == CXType_Pointer) {
            parameters[i].is_ptr = true;
            ASSERT(i != numArgs - 1, "last parameter can't be a pointer!");

            const CXType ele_type = clang_getPointeeType(type);
            const auto spell = getTypeSpelling(ele_type);
            ASSERT(valid_element_type.count(spell) != 0,
                   "Invalid element type: %s", spell.c_str());
            ASSERT(next_arg_name == arg_name + "_len",
                   "Invalid array length name: %s, should be %s_len",
                   next_arg_name.c_str(), next_arg_name.c_str());
        }
        else {
            parameters[i].is_ptr = false;
        }
        next_arg_name = arg_name;
    }

    for (int i = 0; i < numArgs; ++i) {
        CXCursor arg = clang_Cursor_getArgument(cursor, i);

        const auto type = clang_getCursorType(arg);
        /* ASSERT(type.kind != CXType_Pointer, */
        /*        "In your secure function, please use constant array (e.g. char
         * "
         */
        /*        "arr[32]) instead of pointer " */
        /*        "(e.g. char *arr or char arr[])"); */
        bool is_in = true, is_out = true;

        auto &p = parameters[i];
        p.type = getTypeSpelling(type);
        p.name = getCursorSpelling(arg);
        p.array_size = -1;
        p.is_array = type.kind == CXType_ConstantArray;

        do {
            CXType pointee_type;
            if (p.is_array) {
                p.array_size = clang_getArraySize(type);
                pointee_type = clang_getArrayElementType(type);
            }
            else if (p.is_ptr) {
                pointee_type = clang_getPointeeType(type);
            }
            else {
                break;
            }

            auto pointee = getTypeSpelling(pointee_type);

            // TODO: this is just a trick to determine in and out
            p.is_in = pointee == "in_char";
            p.is_out = pointee == "out_char";
            if (p.is_in || p.is_out) {
                pointee = "char";
            }

            p.type = pointee + "*";
        } while (0);
    }
    return parameters;
}

std::string get_function_body(CXCursor cursor, const std::string &filepath)
{
    if (clang_getCursorKind(cursor) != CXCursor_FunctionDecl) {
        return "";
    }

    CXCursor bodyCursor = clang_getNullCursor();
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData clientData) {
            if (clang_getCursorKind(c) == CXCursor_CompoundStmt) {
                *reinterpret_cast<CXCursor *>(clientData) = c;
                return CXChildVisit_Break;
            }
            return CXChildVisit_Continue;
        },
        &bodyCursor);
    if (bodyCursor.kind == CXCursor_InvalidCode) {
        return "";
    }
    CXSourceRange range = clang_getCursorExtent(bodyCursor);
    CXSourceLocation startLoc = clang_getRangeStart(range);
    CXSourceLocation endLoc = clang_getRangeEnd(range);

    unsigned int startOffset, endOffset;
    clang_getSpellingLocation(startLoc, nullptr, nullptr, nullptr,
                              &startOffset);
    clang_getSpellingLocation(endLoc, nullptr, nullptr, nullptr, &endOffset);

    std::string sourceCode = read_file(filepath);

    std::string function_body =
        sourceCode.substr(startOffset, endOffset - startOffset);

    return function_body;
}

// an entry func is a func defined in one world and called in another world
template <WorldType world_type_visited>
CXChildVisitResult entry_func_def_collect_visitor(
    const CXCursor &cursor, const CXCursor &parent,
    const CXClientData &clientData)
{
    const FileContext &file_ctx =
        *reinterpret_cast<const FileContext *>(clientData);

    auto kind = clang_getCursorKind(cursor);
    // ignore headers
    if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0) {
        return CXChildVisit_Continue;
    }

    if (kind == CXCursor_FunctionDecl) {
        auto func_name = getCursorSpelling(cursor);
        DTEE_LOG("VISITING %s IN %s WORLD\n", func_name.c_str(), world_type_visited == WorldType::INSECURE_WORLD ? "INSECURE" : "SECURE");

        bool is_def_valid;

        // the def must be called in another world to be an entry
        if constexpr (world_type_visited == WorldType::SECURE_WORLD) {
            is_def_valid = g_func_calls_in_insecure_world.count(func_name) != 0;
        }
        else {
            is_def_valid = g_func_calls_in_secure_world.count(func_name) != 0;
        }
        static const std::unordered_set<std::string> skip_func_name = {
            "operator new", "operator delete", "operator new[]",
            "operator delete[]"};
        if (skip_func_name.count(func_name) != 0) {
            is_def_valid = false;
        }

        // insecure world can't call a static secure func in secure world!
        // so we can simply ignore static functions
        if (CX_SC_Static != clang_Cursor_getStorageClass(cursor) &&
            is_def_valid) {
            FunctionInfo funcInfo;
            funcInfo.name = std::move(func_name);
            funcInfo.returnType = getFunctionReturnType(cursor);
            funcInfo.parameters = getFunctionParameters(cursor);
            funcInfo.body = get_function_body(cursor, file_ctx.file_path);

            // if body is not empty, then it is a definition
            if (!funcInfo.body.empty()) {
                tls_func_list_each_file.push_back(std::move(funcInfo));
            }
        }
    }
    return CXChildVisit_Recurse;
}

CXChildVisitResult insecure_world_entry_func_def_collect_visitor(
    CXCursor cursor, CXCursor parent, CXClientData clientData)
{
    return entry_func_def_collect_visitor<WorldType::INSECURE_WORLD>(
        cursor, parent, clientData);
}

CXChildVisitResult secure_world_entry_func_def_collect_visitor(
    CXCursor cursor, CXCursor parent, CXClientData clientData)
{
    return entry_func_def_collect_visitor<WorldType::SECURE_WORLD>(
        cursor, parent, clientData);
}

CXChildVisitResult func_call_collect_visitor(CXCursor cursor, CXCursor parent,
                                             CXClientData clientData)
{
    // take func decl as func call because you must define the func before you
    // call it. This is sound, because some function maybe defined but not
    // called, thus taken into account. But that's still correct.
    auto kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_FunctionDecl) {
        tls_func_calls_each_file.insert(getCursorSpelling(cursor));
    }
    if (kind == CXCursor_CallExpr) {
        cursor = clang_getCursorReferenced(cursor);
        if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl) {
            tls_func_calls_each_file.insert(getCursorSpelling(cursor));
        }
    }
    return CXChildVisit_Recurse;
}

struct TranslationUnitManager
{
    CXCursor get_cursor(const std::string &file_path)
    {
        {
            std::shared_lock<std::shared_mutex> lock(rw_mutex);
            auto it = map.find(file_path);
            if (it != map.end()) {
                return clang_getTranslationUnitCursor(it->second.second);
            }
        }

        CXIndex index = clang_createIndex(0, 0);
        const char *args[] = {
            "-fsyntax-only"
            "-E"
        };
        CXTranslationUnit unit = clang_parseTranslationUnit(
            index, file_path.c_str(), args, sizeof(args) / sizeof(*args),
            nullptr, 0, CXTranslationUnit_None);

        if (unit == nullptr) {
            std::cerr << "Unable to parse translation unit: " << file_path
                      << std::endl;
            return {};
        }

        std::scoped_lock<std::shared_mutex> lock(rw_mutex);
        auto it = map.find(file_path);
        if (it != map.end()) {
            return clang_getTranslationUnitCursor(it->second.second);
        }

        map.emplace(file_path, std::make_pair(index, unit));

        CXCursor cursor = clang_getTranslationUnitCursor(map[file_path].second);
        return cursor;
    }

    ~TranslationUnitManager()
    {
        for (auto &[_, pair] : map) {
            /* clang_disposeTranslationUnit(pair.second); */
            /* clang_disposeIndex(pair.first); */
        }
    }

    std::shared_mutex rw_mutex;
    std::unordered_map<std::string, std::pair<CXIndex, CXTranslationUnit>> map;

} manager;

void parse_file(const FileContext &file_ctx, VISITOR visitor)
{
    CXCursor cursor = manager.get_cursor(file_ctx.file_path.c_str());

    clang_visitChildren(cursor, visitor, (void *)&file_ctx);
}

std::string read_file_content(const std::string &filename)
{
    std::ifstream ifs(filename);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}
