#pragma once
#include "pch.h"

struct SourceContext {
  std::string project;
  std::string src_content;
  std::string ret;
  std::string params;
  std::string comma_params;
  std::string comma_param_names;
  std::string edl_params;
  std::string func_name;
  std::string root_cmake;
  std::string host_secure_cmake;
  std::string src_path;
};

std::string parse_template(const std::string &templ, const SourceContext &ctx);

/// @breif each template will generate one file
/// @param ifs input file stream of template file
/// @param context used to replace fields in the template
void generate_with_template(
    const std::filesystem::path &template_path, const SourceContext &ctx,
    const std::filesystem::path &target_path = "generated");
