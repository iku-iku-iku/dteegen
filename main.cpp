#include "pch.h"

#include "fs.h"
#include "parser.h"
std::string toLowerCase(const std::string &str) {
  std::string lowerCaseStr = str;
  std::transform(lowerCaseStr.begin(), lowerCaseStr.end(), lowerCaseStr.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return lowerCaseStr;
}

void replaceAddExecutable(const std::string &filename) {
  std::ifstream fileIn(filename);
  if (!fileIn.is_open()) {
    std::cerr << "无法打开文件: " << filename << std::endl;
    return;
  }

  std::stringstream buffer;
  buffer << fileIn.rdbuf();
  std::string content = buffer.str();
  fileIn.close();

  // 将整个文件内容转换为小写
  std::string contentLower = toLowerCase(content);

  // 查找和替换（大小写不敏感）
  size_t index = 0;
  std::string searchText = "add_executable";
  std::string replaceText = "tee_add_executable";

  while ((index = contentLower.find(searchText, index)) != std::string::npos) {
    content.replace(index, searchText.length(), replaceText);
    contentLower.replace(index, searchText.length(), replaceText);
    index += replaceText.length();
  }

  std::ofstream fileOut(filename);
  if (!fileOut.is_open()) {
    std::cerr << "无法写入文件: " << filename << std::endl;
    return;
  }

  fileOut << content;
  fileOut.close();
}

void generate_secgear(const std::filesystem::path project_root) {

  std::filesystem::path generated_path("generated");
  const auto generated_host = generated_path / HOST;
  const auto generated_enclave = generated_path / ENCLAVE;
  const auto secure_root = project_root / SECURE;
  const auto insecure_root = project_root / INSECURE;

  // remove generated first
  if (std::filesystem::exists(generated_path)) {
    std::filesystem::remove_all(generated_path);
  }

  SourceContext ctx;
  ctx.project = project_root.filename();

  for_each_file_in_dir_recursive(insecure_root, [&](const auto &insecure_file) {
    parse_file(insecure_file.path().c_str(), func_call_collect_visitor,
               nullptr);
    func_calls_in_insecure_world.insert(func_calls_each_file.begin(),
                                        func_calls_each_file.end());
    func_calls_each_file.clear();
  });

  for_each_file_in_dir_recursive(secure_root, [&](const auto &secure_file) {
    parse_file(secure_file.path().c_str(), func_call_collect_visitor, nullptr);
    func_calls_in_secure_world.insert(func_calls_each_file.begin(),
                                      func_calls_each_file.end());
    func_calls_each_file.clear();
  });

  std::cout << "func_calls_in_insecure_world:" << std::endl;
  for (const auto &func : func_calls_in_insecure_world) {
    std::cout << func << std::endl;
  }
  std::cout << "func_calls_in_secure_world:" << std::endl;
  for (const auto &func : func_calls_in_secure_world) {
    std::cout << func << std::endl;
  }

  for_each_file_in_dir_recursive(
      secure_root, [&](const auto &secure_func_file) {
        const auto secure_func_filepath = secure_func_file.path();
        parse_file(secure_func_filepath.c_str(),
                   secure_world_func_def_collect_visitor, nullptr);

        // not contain definition of secure func
        if (func_list_each_file.empty()) {
          const auto new_path =
              std::filesystem::path("generated") / ENCLAVE /
              (secure_func_filepath.lexically_relative(project_root));
          std::filesystem::create_directories(new_path.parent_path());
          std::filesystem::copy_file(
              secure_func_filepath, new_path,
              std::filesystem::copy_options::overwrite_existing);
          return;
        }

        ctx.src_path = secure_func_filepath.lexically_relative(project_root);
        ctx.src = secure_func_filepath.stem().string();
        ctx.src_content = read_file_content(secure_func_filepath);

        for (const auto &e : std::filesystem::recursive_directory_iterator(
                 "template/secure_func_template")) {
          if (e.is_directory()) {
            continue;
          }
          std::ifstream ifs(e.path());

          process_template(ifs, ctx);
        }

        secure_entry_func_list.insert(secure_entry_func_list.end(),
                                      func_list_each_file.begin(),
                                      func_list_each_file.end());
        func_list_each_file.clear();
      });

  for_each_file_in_dir_recursive(
      insecure_root, [&](const auto &insecure_func_file) {
        const auto insecure_func_filepath = insecure_func_file.path();
        parse_file(insecure_func_filepath.c_str(),
                   insecure_world_func_def_collect_visitor, nullptr);

        // not contain definition of secure func
        if (func_list_each_file.empty()) {
          return;
        }

        ctx.src_path = insecure_func_filepath.lexically_relative(project_root);
        ctx.src = insecure_func_filepath.stem().string();
        ctx.src_content = read_file_content(insecure_func_filepath);

        for (const auto &e : std::filesystem::recursive_directory_iterator(
                 "template/insecure_func_template")) {
          if (e.is_directory()) {
            continue;
          }
          std::ifstream ifs(e.path());

          process_template(ifs, ctx);
        }

        insecure_entry_func_list.insert(insecure_entry_func_list.end(),
                                        func_list_each_file.begin(),
                                        func_list_each_file.end());
        func_list_each_file.clear();
      });

  const auto root_cmake_path = project_root / "CMakeLists.txt";
  if (std::filesystem::exists(root_cmake_path)) {
    ctx.root_cmake = read_file_content(root_cmake_path);
  }

  for_each_file_in_dir_recursive("template/project_template",
                                 [&](const auto &f) {
                                   std::ifstream ifs(f.path());

                                   process_template(ifs, ctx);
                                 });

  replaceAddExecutable("generated/host/CMakeLists.txt");

  // copy to host
  for_each_file_in_dir(project_root, [&](const auto &f) {
    const auto new_path =
        generated_host / f.path().lexically_relative(project_root);
    std::filesystem::copy_options op;
    op |= std::filesystem::copy_options::skip_existing;
    std::filesystem::copy_file(f.path(), new_path, op);
  });

  for_each_file_in_dir_recursive(secure_root, [&](const auto &f) {
    const auto new_path =
        generated_host / f.path().lexically_relative(project_root);
    std::filesystem::create_directories(new_path.parent_path());
    std::filesystem::copy_options op;
    op |= std::filesystem::copy_options::skip_existing;
    std::filesystem::copy_file(f.path(), new_path, op);
  });

  for_each_file_in_dir_recursive(insecure_root, [&](const auto &f) {
    const auto new_path =
        generated_host / f.path().lexically_relative(project_root);
    std::filesystem::create_directories(new_path.parent_path());
    std::filesystem::copy_options op;
    op |= std::filesystem::copy_options::skip_existing;
    std::filesystem::copy_file(f.path(), new_path, op);
  });

  // copy enclave
  for_each_file_in_dir_recursive(insecure_root, [&](const auto &f) {
    if (!(f.path().extension() == ".h")) {
      return;
    }
    const auto new_path =
        generated_enclave / f.path().lexically_relative(project_root);
    std::filesystem::create_directories(new_path.parent_path());
    std::filesystem::copy_options op;
    op |= std::filesystem::copy_options::skip_existing;
    std::filesystem::copy_file(f.path(), new_path, op);
  });
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
