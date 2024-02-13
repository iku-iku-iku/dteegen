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
