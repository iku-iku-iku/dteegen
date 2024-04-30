#pragma once

#include <filesystem>

#define TEMPLATE "template"
#define SECURE_FUNC_TEMPLATE_PATH (TEMPLATE "/secure_func_template")
#define INSECURE_FUNC_TEMPLATE_PATH (TEMPLATE "/insecure_func_template")
#define PROJECT_TEMPLATE_PATH (TEMPLATE "/project_template")
#define TEMPLATE_PROJECT_PATH (TEMPLATE "/template_project")
#define TEE_CAPABILITY_PATH (TEMPLATE "/TEE-Capability")
#define SECURE "secure"
#define INSECURE "insecure"
#define ENCLAVE "enclave"
#define HOST "host"
#define SECURE_LIB "secure_lib"
#define ENCLAVE_LIB "enclave_lib"
#define SECURE_INCLUDE "secure_include"
#define ENCLAVE_INCLUDE "enclave_include"

struct PathContext
{
    using Path = std::filesystem::path;

    PathContext(Path project_root)
    {
        project_root_ = project_root;
        project_build_ = project_root_ / "build";
        generated_path_ = "generated";
        template_path_ = std::filesystem::path(TEMPLATE);
        generated_host_ = generated_path_ / HOST;
        generated_enclave_ = generated_path_ / ENCLAVE;
        secure_root_ = project_root_ / SECURE;
        insecure_root_ = project_root_ / INSECURE;
        project_secure_lib_ = secure_root_ / SECURE_LIB;
        generated_enclave_lib_ = generated_enclave_ / ENCLAVE_LIB;
        project_secure_include_ = secure_root_ / SECURE_INCLUDE;
        generated_enclave_include_ = generated_enclave_ / ENCLAVE_INCLUDE;
        insecure_func_template_path_ =
            template_path_ / "insecure_func_template";
        secure_func_template_path_ = template_path_ / "secure_func_template";
        project_template_path_ = template_path_ / "project_template";
        project_root_cmake_path_ = project_root_ / "CMakeLists.txt";
        insecure_root_cmake_path_ = insecure_root_ / "CMakeLists.txt";
        secure_root_cmake_path_ = secure_root_ / "CMakeLists.txt";
    }

    Path project_root_;
    Path template_path_;
    Path generated_path_;
    Path generated_host_;
    Path generated_enclave_;
    Path secure_root_;
    Path insecure_root_;
    Path project_secure_lib_;
    Path generated_enclave_lib_;
    Path project_secure_include_;
    Path generated_enclave_include_;
    Path insecure_func_template_path_;
    Path secure_func_template_path_;
    Path project_template_path_;
    Path project_root_cmake_path_;
    Path insecure_root_cmake_path_;
    Path secure_root_cmake_path_;
    Path project_build_;
};