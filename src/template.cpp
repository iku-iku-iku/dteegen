#include "template.h"

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
