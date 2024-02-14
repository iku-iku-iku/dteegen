#include "pch.h"
#include "pipe/cmake_transform.h"

#include "fs.h"
#include "parser.h"

constexpr auto SKIP_COPY_OPTION = std::filesystem::copy_options::skip_existing;
constexpr auto DIRECTORY_COPY_OPTION =
    std::filesystem::copy_options::recursive |
    std::filesystem::copy_options::overwrite_existing;
void generate_secgear(const std::filesystem::path project_root) {

  std::filesystem::path generated_path("generated");
  std::filesystem::path template_path("template");
  const auto generated_host = generated_path / HOST;
  const auto generated_enclave = generated_path / ENCLAVE;
  const auto secure_root = project_root / SECURE;
  const auto insecure_root = project_root / INSECURE;
  const auto project_secure_lib = secure_root / SECURE_LIB;
  const auto generated_enclave_lib = generated_enclave / ENCLAVE_LIB;
  const auto project_secure_include = secure_root / SECURE_INCLUDE;
  const auto generated_enclave_include = generated_enclave / ENCLAVE_INCLUDE;
  const auto insecure_func_template_path =
      template_path / "insecure_func_template";
  const auto secure_func_template_path = template_path / "secure_func_template";
  const auto project_template_path = template_path / "project_template";
  const auto project_root_cmake_path = project_root / "CMakeLists.txt";
  const auto insecure_root_cmake_path = insecure_root / "CMakeLists.txt";
  const auto secure_root_cmake_path = secure_root / "CMakeLists.txt";

  // remove generated first
  if (std::filesystem::exists(generated_path)) {
    std::filesystem::remove_all(generated_path);
  }

  SourceContext ctx;
  ctx.project = project_root.filename();

  // collect all func calls in secure world and insecure world. for simplicity,
  // we consider declaration as call, since you must decalare before call
  // This assumption will not omit 'true call'
  std::unordered_set<std::string> skip_dir = {"secure_lib", "secure_include"};

  for_each_file_in_path_recursive(
      insecure_root,
      [&](const auto &insecure_file) {
        const auto insecure_file_path = insecure_file.path();
        if (!is_source_file(insecure_file_path)) {
          return;
        }
        // parse insecure file to collect func calls
        FileContext f_ctx{.file_path = insecure_file_path.string()};
        parse_file(f_ctx, func_call_collect_visitor);
        func_calls_in_insecure_world.insert(func_calls_each_file.begin(),
                                            func_calls_each_file.end());
        func_calls_each_file.clear();
      },
      skip_dir);

  for_each_file_in_path_recursive(
      secure_root,
      [&](const auto &secure_file) {
        const auto secure_file_path = secure_file.path();

        if (!is_source_file(secure_file_path)) {
          return;
        }
        // parse secure file to collect func calls
        FileContext f_ctx{.file_path = secure_file_path.string()};
        parse_file(f_ctx, func_call_collect_visitor);
        func_calls_in_secure_world.insert(func_calls_each_file.begin(),
                                          func_calls_each_file.end());
        func_calls_each_file.clear();
      },
      skip_dir);

  for_each_file_in_path_recursive(
      secure_root,
      [&](const auto &secure_func_file) {
        const auto secure_func_filepath = secure_func_file.path();

        if (!is_source_file(secure_func_filepath)) {
          return;
        }
        // collect all secure entry func definition in secure func file
        FileContext f_ctx{.file_path = secure_func_filepath.string()};
        parse_file(f_ctx, secure_world_entry_func_def_collect_visitor);

        // if the secure file doesn't contain definition of secure entry func,
        // then it's just a normal file, e.g. header file
        if (func_list_each_file.empty()) {
          const auto new_path =
              generated_enclave /
              (secure_func_filepath.lexically_relative(project_root));
          std::filesystem::create_directories(new_path.parent_path());

          // simply copy no-def file to enclave, cause it maybe needed.
          std::filesystem::copy_file(
              secure_func_filepath, new_path,
              std::filesystem::copy_options::overwrite_existing);
          return;
        }

        ctx.src_path = secure_func_filepath.lexically_relative(project_root);
        ctx.src_content = read_file_content(secure_func_filepath);

        // process secure func template for funcs in this file
        for_each_file_in_path_recursive(secure_func_template_path,
                                        [&](const auto &e) {
                                          std::ifstream ifs(e.path());

                                          process_template(ifs, ctx);
                                        });

        secure_entry_func_list.insert(secure_entry_func_list.end(),
                                      func_list_each_file.begin(),
                                      func_list_each_file.end());
        func_list_each_file.clear();
      },
      skip_dir);

  for_each_file_in_path_recursive(
      insecure_root,
      [&](const auto &insecure_func_file) {
        const auto insecure_func_filepath = insecure_func_file.path();
        if (!is_source_file(insecure_func_filepath)) {
          return;
        }

        FileContext f_ctx{.file_path = insecure_func_filepath.string()};
        parse_file(f_ctx, insecure_world_entry_func_def_collect_visitor);

        // not contain definition of insecure entry func
        if (func_list_each_file.empty()) {
          return;
        }

        ctx.src_path = insecure_func_filepath.lexically_relative(project_root);
        ctx.src_content = read_file_content(insecure_func_filepath);

        for_each_file_in_path_recursive(insecure_func_template_path,
                                        [&](const auto &e) {
                                          std::ifstream ifs(e.path());
                                          process_template(ifs, ctx);
                                        });

        insecure_entry_func_list.insert(insecure_entry_func_list.end(),
                                        func_list_each_file.begin(),
                                        func_list_each_file.end());
        func_list_each_file.clear();
      },
      skip_dir);

  // process project level template now
  if (std::filesystem::exists(insecure_root_cmake_path)) {
    ctx.root_cmake = read_file_content(insecure_root_cmake_path);
  }

  if (std::filesystem::exists(secure_root_cmake_path)) {
    ctx.host_secure_cmake = read_file_content(secure_root_cmake_path);
  }

  for_each_file_in_path_recursive(project_template_path, [&](const auto &f) {
    std::ifstream ifs(f.path());
    process_template(ifs, ctx);
  });

  // copy remaining files in secure world to enclave
  for_each_file_in_path_recursive(
      secure_root,
      [&](const auto &f) {
        const auto new_path =
            generated_enclave / f.path().lexically_relative(project_root);
        std::filesystem::create_directories(new_path.parent_path());
        std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION);
      },
      skip_dir);

  // copy headers in insecure world to enclave, cause insecure world can include
  // them
  for_each_file_in_path_recursive(insecure_root, [&](const auto &f) {
    if (!(f.path().extension() == ".h")) {
      return;
    }
    const auto new_path =
        generated_enclave / f.path().lexically_relative(project_root);
    std::filesystem::create_directories(new_path.parent_path());
    std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION);
  });

  // copy enclave libs
  if (std::filesystem::exists(project_secure_lib)) {
    std::filesystem::create_directories(generated_enclave_lib);
    for (const auto &f :
         std::filesystem::directory_iterator(project_secure_lib)) {
      const auto relative_path =
          f.path().lexically_relative(project_secure_lib);

      std::filesystem::copy(f.path(), generated_enclave_lib / relative_path,
                            DIRECTORY_COPY_OPTION);
    }
  }

  // copy enclave includes
  if (std::filesystem::exists(project_secure_include)) {
    std::filesystem::create_directories(generated_enclave_include);
    for (const auto &f :
         std::filesystem::directory_iterator(project_secure_include)) {
      const auto relative_path =
          f.path().lexically_relative(project_secure_include);

      std::filesystem::copy(f.path(), generated_enclave_include / relative_path,
                            DIRECTORY_COPY_OPTION);
    }
  }

  /* // copy remaining files in secure world to host */
  /* for_each_file_in_path_recursive(secure_root, [&](const auto &f) { */
  /*   const auto new_path = */
  /*       generated_host / f.path().lexically_relative(project_root); */
  /*   std::filesystem::create_directories(new_path.parent_path()); */
  /*   std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION); */
  /* }); */
  /**/
  /* // copy remaining files in insecure world to host */
  /* for_each_file_in_path_recursive(insecure_root, [&](const auto &f) { */
  /*   const auto new_path = */
  /*       generated_host / f.path().lexically_relative(project_root); */
  /*   std::filesystem::create_directories(new_path.parent_path()); */
  /*   std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION); */
  /* }); */

  // copy remaining files in project root to host
  for_each_file_in_path_recursive(project_root, [&](const auto &f) {
    const auto new_path =
        generated_host / f.path().lexically_relative(project_root);
    std::filesystem::create_directories(new_path.parent_path());
    std::filesystem::copy_file(f.path(), new_path, SKIP_COPY_OPTION);
  });

  replace_case_insensitive(generated_host / SECURE / "CMakeLists.txt",
                           "add_library", "tee_add_library");
}

// 主函数
int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " project\n";
    return 1;
  }
  // measure time
  auto start = std::chrono::high_resolution_clock::now();
  generate_secgear(argv[1]);
  auto end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                  .count();
  std::cout << "Time: " << time << "ms" << std::endl;
  return 0;
}
