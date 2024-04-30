#include <filesystem>

#include "collector.h"
#include "elf_parser.h"
#include "fs.h"
#include "parser.h"
#include "path_context.h"
#include "pch.h"
#include "pipe/cmake_transform.h"
#include "template.h"

const auto relative_path(const std::string &path, const std::string &base)
{
    const auto n = path.size(), m = base.size();
    return path.substr(m + 1, n - m);
}
#define POOL_SIZE 1

constexpr auto SKIP_COPY_OPTION = std::filesystem::copy_options::skip_existing;
constexpr auto DIRECTORY_COPY_OPTION =
    std::filesystem::copy_options::recursive |
    std::filesystem::copy_options::overwrite_existing;
void generate_secgear(const std::filesystem::path project_root)
{
    PathContext path_ctx(project_root);
    const auto elfs = collect_elf(path_ctx);
    // remove generated first
    if (std::filesystem::exists(path_ctx.generated_path_)) {
        std::filesystem::remove_all(path_ctx.generated_path_);
    }

    // collect all func calls in secure world and insecure world. for
    // simplicity, we consider declaration as call, since you must decalare
    // before call This assumption will not omit 'true call'
    std::unordered_set<std::string> skip_dir = {"secure_lib", "secure_include"};

    ThreadPool pool(POOL_SIZE);
    DTEE_LOG("Created thread pool with size: %d\n", POOL_SIZE);
    SourceContext ctx;
    ctx.project = project_root.filename();

    DTEE_LOG("BEGIN COLLECT FUNC CALL\n");

    for_each_file_in_path_recursive_parallel(
        path_ctx.insecure_root_, ElfCollector<WorldType::INSECURE_WORLD>(elfs),
        skip_dir, pool);

    for_each_file_in_path_recursive_parallel(
        path_ctx.secure_root_, ElfCollector<WorldType::SECURE_WORLD>(elfs),
        skip_dir, pool);

    pool.wait_queue_empty();
    for (const auto &e : g_func_calls_in_insecure_world) {
        DTEE_LOG("FUNC CALL IN INSECURE WORLD: %s\n", e.c_str());
    }
    // for (const auto &e : g_func_calls_in_secure_world) {
    //     DTEE_LOG("FUNC CALL IN SECURE WORLD: %s\n", e.c_str());
    // }
    DTEE_LOG("END COLLECT FUNC CALL\n");

    std::mutex fs_mutex;
    const auto process_secure_file = [&](const auto secure_func_file) {
        const auto secure_func_filepath = secure_func_file.path();
        if (!is_source_file(secure_func_filepath)) {
            return;
        }

        DTEE_LOG("BEGIN PROCESS SECURE FILE: %s\n",
                 secure_func_file.path().c_str());
        // collect all secure entry func definition in secure func file
        FileContext f_ctx{.file_path = secure_func_filepath.string()};
        parse_file(f_ctx, secure_world_entry_func_def_collect_visitor);

        // if the secure file doesn't contain definition of secure entry func,
        // then it's just a normal file, e.g. header file
        if (tls_func_list_each_file.empty()) {
            /* const auto new_path = */
            /*     generated_enclave / */
            /*     (secure_func_filepath.lexically_relative(project_root)); */
            /* std::filesystem::create_directories(new_path.parent_path()); */
            /**/
            /* // simply copy no-def file to enclave, cause it maybe needed. */
            /* std::filesystem::copy_file( */
            /*     secure_func_filepath, new_path, */
            /*     std::filesystem::copy_options::overwrite_existing); */
            DTEE_LOG("END PROCESS SECURE FILE: %s (no entry func found)\n",
                     secure_func_file.path().c_str());
            return;
        }
        else {
            for (const auto &f : tls_func_list_each_file) {
                DTEE_LOG("FOUND secure entry func: %s\n", f.name.c_str());
            }
        }

        SourceContext ctx;
        ctx.project = project_root.filename();
        ctx.src_path = relative_path(secure_func_filepath, project_root);
        ctx.src_content = read_file_content(secure_func_filepath);

        // process secure func template for funcs in this file
        for_each_file_in_path_recursive(
            path_ctx.secure_func_template_path_,
            [&](const auto &e) { generate_with_template(e.path(), ctx); });

        tls_secure_entry_func_list.insert(tls_secure_entry_func_list.end(),
                                          tls_func_list_each_file.begin(),
                                          tls_func_list_each_file.end());
        tls_func_list_each_file.clear();
        DTEE_LOG("END PROCESS SECURE FILE: %s\n",
                 secure_func_file.path().c_str());
    };

    const auto process_insecure_file = [&, project_root](
                                           const auto &insecure_func_file) {
        const auto insecure_func_filepath = insecure_func_file.path();
        if (!is_source_file(insecure_func_filepath)) {
            return;
        }

        DTEE_LOG("BEGIN PROCESS INSECURE FILE: %s\n",
                 insecure_func_file.path().c_str());
        FileContext f_ctx{.file_path = insecure_func_filepath.string()};
        parse_file(f_ctx, insecure_world_entry_func_def_collect_visitor);

        // not contain definition of insecure entry func
        if (tls_func_list_each_file.empty()) {
            DTEE_LOG("END PROCESS INSECURE FILE: %s (no entry func found)\n",
                     insecure_func_file.path().c_str());
            return;
        }

        SourceContext ctx;
        ctx.project = project_root.filename();
        ctx.src_path = relative_path(insecure_func_filepath, project_root);
        ctx.src_content = read_file_content(insecure_func_filepath);

        for_each_file_in_path_recursive(
            path_ctx.insecure_func_template_path_,
            [&](const auto &e) { generate_with_template(e.path(), ctx); });

        tls_insecure_entry_func_list.insert(tls_insecure_entry_func_list.end(),
                                            tls_func_list_each_file.begin(),
                                            tls_func_list_each_file.end());
        tls_func_list_each_file.clear();
        DTEE_LOG("END PROCESS INSECURE FILE: %s\n",
                 insecure_func_file.path().c_str());
    };

    for_each_file_in_path_recursive_parallel(
        path_ctx.secure_root_, process_secure_file, skip_dir, pool);

    for_each_file_in_path_recursive_parallel(
        path_ctx.insecure_root_, process_insecure_file, skip_dir, pool);

    pool.wait_queue_empty();

    DTEE_LOG("process project level template now\n");
    if (std::filesystem::exists(path_ctx.insecure_root_cmake_path_)) {
        ctx.root_cmake = read_file_content(path_ctx.insecure_root_cmake_path_);
    }

    if (std::filesystem::exists(path_ctx.secure_root_cmake_path_)) {
        ctx.host_secure_cmake =
            read_file_content(path_ctx.secure_root_cmake_path_);
    }

    for_each_file_in_path_recursive(
        path_ctx.project_template_path_,
        [&](const auto &f) { generate_with_template(f.path(), ctx); });

    // copy remaining files in secure world to enclave
    for_each_file_in_path_recursive(
        path_ctx.secure_root_,
        [&](const auto &f) {
            const auto new_path = path_ctx.generated_enclave_ /
                                  f.path().lexically_relative(project_root);
            std::filesystem::create_directories(new_path.parent_path());
            std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION);
        },
        skip_dir);

    // copy headers in insecure world to enclave, cause insecure world can
    // include them
    for_each_file_in_path_recursive(
        path_ctx.insecure_root_, [&](const auto &f) {
            if (!(f.path().extension() == ".h")) {
                return;
            }
            const auto new_path = path_ctx.generated_enclave_ /
                                  f.path().lexically_relative(project_root);
            std::filesystem::create_directories(new_path.parent_path());
            std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION);
        });

    // copy enclave libs
    if (std::filesystem::exists(path_ctx.project_secure_lib_)) {
        std::filesystem::create_directories(path_ctx.generated_enclave_lib_);
        for (const auto &f : std::filesystem::directory_iterator(
                 path_ctx.project_secure_lib_)) {
            const auto relative_path =
                f.path().lexically_relative(path_ctx.project_secure_lib_);

            std::filesystem::copy(
                f.path(), path_ctx.generated_enclave_lib_ / relative_path,
                DIRECTORY_COPY_OPTION);
        }
    }

    // copy enclave includes
    if (std::filesystem::exists(path_ctx.project_secure_include_)) {
        std::filesystem::create_directories(
            path_ctx.generated_enclave_include_);
        for (const auto &f : std::filesystem::directory_iterator(
                 path_ctx.project_secure_include_)) {
            const auto relative_path =
                f.path().lexically_relative(path_ctx.project_secure_include_);

            std::filesystem::copy(
                f.path(), path_ctx.generated_enclave_include_ / relative_path,
                DIRECTORY_COPY_OPTION);
        }
    }

    // copy remaining files in project root to host
    for_each_file_in_path_recursive(path_ctx.project_root_, [&](const auto &f) {
        const auto new_path =
            path_ctx.generated_host_ /
            f.path().lexically_relative(path_ctx.project_root_);
        std::filesystem::create_directories(new_path.parent_path());
        std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION);
    });

    replace_case_insensitive(
        path_ctx.generated_host_ / SECURE / "CMakeLists.txt", "add_library",
        "tee_add_library");
}

void convert(std::string project_path) { generate_secgear(project_path); }
void create(const char *project_path)
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

// 主函数
int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [create]/[convert]"
                  << " [project_path]\n";
        return 1;
    }

    // measure time
    auto start = std::chrono::high_resolution_clock::now();
    if (!strcmp(argv[1], "create")) {
        create(argv[2]);
    }
    else if (!strcmp(argv[1], "convert")) {
        convert(argv[2]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    std::cout << "Time: " << time << "ms" << std::endl;
    return 0;
}
