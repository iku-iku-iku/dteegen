#pragma once

#include <filesystem>

#include "elf_parser.h"
#include "fs.h"
#include "parser.h"
#include "path_context.h"

template <WorldType WorldType>
struct NaiveCollector
{
    void operator()(const std::filesystem::directory_entry& dentry)
    {
        const auto path = dentry.path();
        if (!is_source_file(path)) {
            return;
        }
        // parse insecure file to collect func calls
        FileContext file_context{.file_path = path.string()};
        parse_file(file_context, func_call_collect_visitor);

        if constexpr (WorldType == WorldType::SECURE_WORLD) {
            tls_func_calls_in_secure_world.insert(
                tls_func_calls_each_file.begin(),
                tls_func_calls_each_file.end());
        }
        else {
            tls_func_calls_in_insecure_world.insert(
                tls_func_calls_each_file.begin(),
                tls_func_calls_each_file.end());
        }
        tls_func_calls_each_file.clear();
    }
};

template <WorldType WorldType>
struct ElfCollector
{
    const std::vector<std::string>& elfs_;

    ElfCollector(const std::vector<std::string>& elfs) : elfs_(elfs)
    {
    }

    void operator()(const std::filesystem::directory_entry& filename)
    {
        const auto path = filename.path();

        bool found = false;
        std::string elf_path;
        for (const auto& elf : elfs_) {
            if (elf.find(path.filename().string()) == std::string::npos) {
                continue;
            }
            found = true;
            elf_path = elf;
        }

        if (!found) {
            return;
        }

        symbols syms;
        parse_symbols(elf_path, syms);

// std::cout << path.filename().string() << std::endl;
//         for (const auto& sym : syms.defined) {
//             std::cout << "SYM: " << sym << std::endl;
//         }
        if constexpr (WorldType == WorldType::SECURE_WORLD) {
            tls_func_calls_in_secure_world.insert(syms.undefined.begin(),
                                                  syms.undefined.end());
        }
        else {
            tls_func_calls_in_insecure_world.insert(syms.undefined.begin(),
                                                    syms.undefined.end());
        }
    }
};

std::vector<std::string> collect_elf(PathContext& path_context);