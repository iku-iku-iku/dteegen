#include <pch.h>

std::string toLowerCase(const std::string &str) {
  std::string lowerCaseStr = str;
  std::transform(lowerCaseStr.begin(), lowerCaseStr.end(), lowerCaseStr.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return lowerCaseStr;
}

void replace_case_insensitive(const std::string &filename,
                              const std::string &old_str,
                              const std::string &new_str) {
  std::ifstream fileIn(filename);
  if (!fileIn.is_open()) {
    std::cerr << "open file failed" << filename << std::endl;
    return;
  }

  std::stringstream buffer;
  buffer << fileIn.rdbuf();
  std::string content = buffer.str();
  fileIn.close();

  std::string contentLower = toLowerCase(content);

  size_t index = 0;
  const std::string &searchText = old_str;
  const std::string &replaceText = new_str;

  while ((index = contentLower.find(searchText, index)) != std::string::npos) {
    content.replace(index, searchText.length(), replaceText);
    contentLower.replace(index, searchText.length(), replaceText);
    index += replaceText.length();
  }

  std::ofstream fileOut(filename);
  if (!fileOut.is_open()) {
    std::cerr << "fail to write file" << filename << std::endl;
    return;
  }

  fileOut << content;
  fileOut.close();
}
