#include "template.h"
#include "parser.h"

/* #define PATTERN(name) \ */
/*   { std::regex("(\\$\\{" #name "\\})"), &SourceContext::name } */

#define PATTERN(name)                                                          \
  { #name, &SourceContext::name }

std::unordered_map<std::string, decltype(&SourceContext::project)> replaces{
    PATTERN(src_content), PATTERN(ret),
    PATTERN(params),      PATTERN(comma_params),
    PATTERN(func_name),   PATTERN(comma_param_names),
    PATTERN(root_cmake),  PATTERN(host_secure_cmake),
    PATTERN(project),     PATTERN(edl_params),
    PATTERN(src_path)};

std::string parse_template(const std::string &templ, const SourceContext &ctx) {
  std::stringstream ss;

  size_t cur = 0, end = templ.size();
  char ch;
  while (cur < end) {
    ch = templ[cur++];
    if (ch == '$') {
      if (cur < end && templ[cur] == '{') {
        size_t start = cur + 1;
        size_t end = templ.find('}', start);
        if (end != std::string::npos) {
          std::string name = templ.substr(start, end - start);
          if (replaces.find(name) != replaces.end()) {
            ss << ctx.*replaces[name];
            cur = end + 1;
            continue;
          }
        }
      }
    }
    ss << ch;
  }
  /* for (const auto &[pat, repl] : replaces) { */
  /*   res = std::regex_replace(res, pat, ctx.*repl); */
  /* } */
  return ss.str();
}

template <bool WithType, bool IsEDL, bool WithCommaAhead>
std::string get_params_str(const std::vector<Param> &params) {
  std::stringstream ss;
  bool flag = false;

  // if without type, the params will be used after other params
  for (const auto &param : params) {
    if constexpr (!WithCommaAhead) {
      if (flag) {
        ss << ", ";
      }
    } else {
      ss << ", ";
    }
    if constexpr (IsEDL) {
      if (param.is_array || param.is_ptr) {
        ss << "[";
        if (param.is_in && !param.is_out) {
          ss << "in";
        } else if (!param.is_in && param.is_out) {
          ss << "out";
        } else {
          ss << "in, out";
        }
        ss << ", size=";
        if (param.is_ptr) {
          ss << param.name << "_len";
        } else {
          ASSERT(param.array_size >= 0, "param size < 0");
          ss << param.array_size;
        }
        ss << "] ";
      }
    }
    if constexpr (WithType) {
      ss << param.type << " " << param.name;
    } else {
      ss << param.name;
    }
    if constexpr (!WithCommaAhead) {
      flag = true;
    }
  }
  return ss.str();
}

std::string get_params(const std::vector<Param> &params) {
  return get_params_str<true, false, false>(params);
}

std::string get_comma_params(const std::vector<Param> &params) {
  return get_params_str<true, false, true>(params);
}

std::string get_comma_param_names(const std::vector<Param> &params) {
  return get_params_str<false, false, true>(params);
}

std::string get_edl_params(const std::vector<Param> &params) {
  return get_params_str<true, true, false>(params);
}

std::string get_filepath(std::ifstream &ifs, const SourceContext &ctx) {
  assert(ifs);
  std::string label, filepath;
  ifs >> label;
  assert(label == "path:");

  ifs >> filepath;
  return parse_template(filepath, ctx);
}

enum class LineType : uint8_t { Begin, GBegin, IGBegin, End, Normal };
LineType get_line_type(const std::string &line) {
  if (line.size() < 3)
    return LineType::Normal;

  if (line[0] != '*' || line[1] != '*')
    return LineType::Normal;

  switch (line[2]) {
  case 'b':
    return LineType::Begin;
  case 'g':
    return LineType::GBegin;
  case 'i':
    return LineType::IGBegin;
  case 'e':
    return LineType::End;
  default:
    return LineType::Normal;
  }

  /* if (line.find("**begin**") != std::string::npos) { */
  /*   return LineType::Begin; */
  /* } else if (line.find("**gbegin**") != std::string::npos) { */
  /*   return LineType::GBegin; */
  /* } else if (line.find("**igbegin**") != std::string::npos) { */
  /*   return LineType::IGBegin; */
  /* } else if (line.find("**end**") != std::string::npos) { */
  /*   return LineType::End; */
  /* } else { */
  /*   return LineType::Normal; */
  /* } */
}

std::string get_content(std::ifstream &ifs, const SourceContext &ctx) {
  assert(ifs);
  std::string line;
  std::stringstream ss;
  bool multi_template = false;
  bool global = false;
  bool insecure = false;
  std::vector<std::string> lines;
  while (std::getline(ifs, line)) {
    const auto line_type = get_line_type(line);
    /* switch (line_type) { */
    /**/
    /* case LineType::Begin: */
    /*   multi_template = true; */
    /*   continue; */
    /* case LineType::GBegin: */
    /*   multi_template = true; */
    /*   global = true; */
    /*   continue; */
    /* case LineType::IGBegin: */
    /*   multi_template = true; */
    /*   global = true; */
    /*   insecure = true; */
    /*   continue; */
    /* case LineType::End: { */
    /*   SourceContext each_ctx = ctx; */
    /**/
    /*   const auto &list = global ? (insecure ? g_insecure_entry_func_list */
    /*                                         : g_secure_entry_func_list) */
    /*                             : tls_func_list_each_file; */
    /**/
    /*   for (const auto &func : list) { */
    /*     each_ctx.func_name = func.name; */
    /*     each_ctx.params = get_params(func.parameters); */
    /*     each_ctx.comma_params = get_comma_params(func.parameters); */
    /*     each_ctx.comma_param_names = get_comma_param_names(func.parameters);
     */
    /*     each_ctx.edl_params = get_edl_params(func.parameters); */
    /*     each_ctx.ret = func.returnType; */
    /**/
    /*     for (const auto &line : lines) { */
    /*       ss << parse_template(line, each_ctx) << std::endl; */
    /*     } */
    /*   } */
    /**/
    /*   lines.clear(); */
    /**/
    /*   multi_template = false; */
    /*   global = false; */
    /*   insecure = false; */
    /*   continue; */
    /* } */
    /* case LineType::Normal: */
    /*   break; */
    /* } */

    if (line.find("**begin**") != std::string::npos) {
      multi_template = true;
      continue;
    } else if (line.find("**gbegin**") != std::string::npos) {
      multi_template = true;
      global = true;
      continue;
    } else if (line.find("**igbegin**") != std::string::npos) {
      multi_template = true;
      global = true;
      insecure = true;
      continue;
    } else if (line.find("**end**") != std::string::npos) {

      SourceContext each_ctx = ctx;

      const auto &list = global ? (insecure ? g_insecure_entry_func_list
                                            : g_secure_entry_func_list)
                                : tls_func_list_each_file;

      for (const auto &func : list) {
        each_ctx.func_name = func.name;
        each_ctx.params = get_params(func.parameters);
        each_ctx.comma_params = get_comma_params(func.parameters);
        each_ctx.comma_param_names = get_comma_param_names(func.parameters);
        each_ctx.edl_params = get_edl_params(func.parameters);
        each_ctx.ret = func.returnType;

        for (const auto &line : lines) {
          ss << parse_template(line, each_ctx) << std::endl;
        }
      }

      lines.clear();

      multi_template = false;
      global = false;
      insecure = false;
      continue;
    }
    if (multi_template) {
      lines.push_back(std::move(line));
    } else {
      ss << parse_template(std::move(line), ctx) << std::endl;
    }
  }
  return ss.str();
}

void generate_with_template(const std::filesystem::path &template_path,
                            const SourceContext &ctx) {
  std::ifstream ifs(template_path);
  const auto filepath = get_filepath(ifs, ctx);
  const auto content = get_content(ifs, ctx);

  const auto path = std::filesystem::path("./generated") / filepath;
  std::cout << "GENERATED:" << path << std::endl;
  std::filesystem::create_directories(path.parent_path());
  std::ofstream ofs(path);
  ofs << content;
}
