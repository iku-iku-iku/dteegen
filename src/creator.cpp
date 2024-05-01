#include "pch.h"
// pch above
#include "creator.h"
#include "fs.h"
#include "path_context.h"

void create(const std::string& project_path)
{
    const auto project_root = std::filesystem::path(project_path);
    if (std::filesystem::exists(project_root)) {
        std::cerr << "Project already exists\n";
        return;
    }

    std::string project_name = project_root.filename();

    for_each_file_in_path_recursive(TEMPLATE_PROJECT_PATH, [&](const auto &f) {
        SourceContext ctx;
        ctx.project = project_name;
        generate_with_template(f.path(), ctx, project_root);
    });

    for_each_in_dir_recurisive(TEMPLATE_PROJECT_PATH, [&](const auto &f) {
        const auto new_path =
            project_root / f.path().lexically_relative(TEMPLATE_PROJECT_PATH);
        if (f.is_directory()) {
            std::filesystem::create_directories(new_path);
        }
    });

    for_each_file_in_path_recursive(TEE_CAPABILITY_PATH, [&](const auto &f) {
        const auto new_path =
            project_root / ".dev" / f.path().lexically_relative(TEMPLATE);
        std::cout << new_path << std::endl;
        std::filesystem::create_directories(new_path.parent_path());
        std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION);
    });
}
