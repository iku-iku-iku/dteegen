
#include "collector.h"

static bool ends_with(const std::string& str, const std::string& suffix)
{
    if (str.length() < suffix.length()) return false;
    return str.substr(str.length() - suffix.length()) == suffix;
}

std::vector<std::string> collect_elf(PathContext& path_context)
{
    if (!std::filesystem::exists(path_context.project_build_)) {
        DTEE_LOG("Project build directory does not exist\n");
        return {};
    }
    std::vector<std::string> res;
    for_each_file_in_path_recursive(
        path_context.project_build_, [&](const auto& dentry) {
            const auto path = dentry.path().string();
            if (ends_with(path, ".o")) {
                res.push_back(path);
            }
        });

    return res;
}