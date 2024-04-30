#include <iostream>
#include <fstream>
#include <memory>
#include <elf.h>
#include <regex>
#include <vector>
#include <cxxabi.h>
#include "elf_parser.h"

std::string extract_func_name(const std::string& signature) {
    std::regex rgx(R"(\b(\w+)\s*\(.*\)$)");
    std::smatch match;
    if (std::regex_search(signature, match, rgx) &&match.size() > 1) {
        return match[1].str();
    }

    return "";
}

std::string demangle_function(const char* mangle_name) {
    int status;
    std::unique_ptr<char, void(*)(void*)> demangle_name(
        abi::__cxa_demangle(mangle_name, nullptr, nullptr, &status),
        std::free
    );

    if (status == 0) {
        std::string res = demangle_name.get();
        res = extract_func_name(res);
        return res;
    } else {
        if (status == -2) {
            return mangle_name;
        }
        return "";
    }
}
void parse_symbols(const std::string& filename, symbols& syms) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    Elf64_Ehdr ehdr;
    file.read((char*)&ehdr, sizeof(ehdr));

    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        std::cerr << "Invalid ELF file." << std::endl;
        return;
    }

    file.seekg(ehdr.e_shoff, std::ios::beg);
    std::vector<Elf64_Shdr> sections(ehdr.e_shnum);
    file.read((char*)sections.data(), ehdr.e_shnum * sizeof(Elf64_Shdr));

    for (const auto& section : sections) {
        if (section.sh_type == SHT_SYMTAB || section.sh_type == SHT_DYNSYM) {
            std::vector<Elf64_Sym> symbols(section.sh_size / sizeof(Elf64_Sym));
            file.seekg(section.sh_offset, std::ios::beg);
            file.read((char*)symbols.data(), section.sh_size);

            Elf64_Shdr strSection = sections[section.sh_link];
            std::vector<char> stringTable(strSection.sh_size);
            file.seekg(strSection.sh_offset, std::ios::beg);
            file.read(stringTable.data(), strSection.sh_size);

            for (const auto& sym : symbols) {
                char symType = ELF64_ST_TYPE(sym.st_info);
                if (symType == STT_FUNC || symType == STT_NOTYPE) {
                    std::string name = &stringTable[sym.st_name];
                    if (sym.st_shndx == SHN_UNDEF) {
                        name = demangle_function(name.c_str());
                        if (name.empty()) { continue; }
                        syms.undefined.insert(name);
                    } else if (sym.st_shndx != SHN_UNDEF && symType == STT_FUNC) {
                        name = demangle_function(name.c_str());
                        if (name.empty()) { continue; }
                        syms.defined.insert(name);
                    }
                }
            }
        }
    }
}
