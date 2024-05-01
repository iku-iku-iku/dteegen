#include <unordered_set>

#include "pch.h"
// pch above
#include "collector.h"
#include "elf_parser.h"
#include "fs.h"
#include "generator.h"
#include "path_context.h"
#include "pipe/cmake_transform.h"
#include "thread_pool.h"

#define POOL_SIZE 4

namespace {
std::unordered_set<std::string> skip_dir = {"secure_lib", "secure_include"};

struct copy_config
{
    std::filesystem::path src;
    std::filesystem::path dst;
    std::filesystem::path rel;
    bool skip = false;
    bool only_header = false;
};

void copy_files(const PathContext &path_ctx)
{
    const std::vector<copy_config> copy_configs = {
        {path_ctx.secure_root_, path_ctx.generated_enclave_,
         path_ctx.project_root_, true},
        {.src = path_ctx.insecure_root_,
         .dst = path_ctx.generated_enclave_,
         .rel = path_ctx.project_root_,
         .only_header = true},
        {path_ctx.project_secure_lib_, path_ctx.generated_enclave_lib_,
         path_ctx.project_secure_lib_},
        {path_ctx.project_secure_include_, path_ctx.generated_enclave_include_,
         path_ctx.project_secure_include_},
        {path_ctx.project_root_, path_ctx.generated_host_,
         path_ctx.project_root_}};
    for (const auto &config : copy_configs) {
        if (!std::filesystem::exists(config.src)) {
            continue;
        }
        const auto skip =
            config.skip ? skip_dir : std::unordered_set<std::string>();
        for_each_file_in_path_recursive(
            config.src,
            [&](const auto &f) {
                if (config.only_header && f.path().extension() != ".h") {
                    return;
                }
                const auto new_path =
                    config.dst / f.path().lexically_relative(config.rel);
                if (!std::filesystem::exists(new_path.parent_path())) {
                    std::filesystem::create_directories(new_path.parent_path());
                }
                std::filesystem::copy_file(f.path(), new_path,
                                           SKIP_COPY_OPTION);
            },
            skip);
    }
}
}  // namespace

void generate_secgear(const std::filesystem::path project_root)
{
    PathContext path_ctx(project_root);
    const auto elfs = collect_elf(path_ctx);
    // remove generated first
    if (std::filesystem::exists(path_ctx.generated_path_)) {
        std::filesystem::remove_all(path_ctx.generated_path_);
    }

    ThreadPool pool(POOL_SIZE);
    DTEE_LOG("Created thread pool with size: %d\n", POOL_SIZE);
    SourceContext ctx;
    ctx.project = project_root.filename();

    DTEE_LOG("BEGIN COLLECT FUNC CALL\n");

    // collect all func calls in secure world and insecure world. We parse elf
    // to find undefined symbols and consider them as func calls.

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
    for (const auto &e : g_func_calls_in_secure_world) {
        DTEE_LOG("FUNC CALL IN SECURE WORLD: %s\n", e.c_str());
    }
    DTEE_LOG("END COLLECT FUNC CALL\n");

    DTEE_LOG("BEGIN GENERATE\n");

    for_each_file_in_path_recursive_parallel(
        path_ctx.secure_root_,
        NaiveGenerator<WorldType::SECURE_WORLD>(path_ctx), skip_dir, pool);

    for_each_file_in_path_recursive_parallel(
        path_ctx.insecure_root_,
        NaiveGenerator<WorldType::INSECURE_WORLD>(path_ctx), skip_dir, pool);

    pool.wait_queue_empty();

    DTEE_LOG("END GENERATE\n");

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

    // copy remaining files
    copy_files(path_ctx);

    replace_case_insensitive(
        path_ctx.generated_host_ / SECURE / "CMakeLists.txt", "add_library",
        "tee_add_library");
}