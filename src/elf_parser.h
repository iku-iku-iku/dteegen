#pragma once

#include <string>
#include <unordered_set>

struct symbols {
    std::unordered_set<std::string> undefined;
    std::unordered_set<std::string> defined;
};


void parse_symbols(const std::string& filename, symbols& syms);