#pragma once

#include "fs.h"
#include "path_context.h"
#include "pch.h"

template <WorldType WorldType>
struct NaiveGenerator
{
    const PathContext& path_context_;

    NaiveGenerator(const PathContext& path_context)
        : path_context_(path_context)
    {
    }

    void operator()(const std::filesystem::directory_entry& dentry)
    {
        const auto path = dentry.path();
        if (!is_source_file(path)) {
            return;
        }

        // collect all secure entry func definition in secure func file
        FileContext file_context{.file_path = path.string()};
        const auto& visitor =
            WorldType == WorldType::SECURE_WORLD
                ? secure_world_entry_func_def_collect_visitor
                : insecure_world_entry_func_def_collect_visitor;
        parse_file(file_context, visitor);

        // if the secure file doesn't contain definition of secure entry func,
        // then it's just a normal file, e.g. header file
        if (tls_func_list_each_file.empty()) {
            return;
        }

        SourceContext source_context{
            .project = path_context_.project_root_.filename(),
            .src_content = read_file_content(path),
            .src_path = relative_path(path, path_context_.project_root_),
        };

        // process (in)secure func template for funcs in this file
        const auto& template_path =
            WorldType == WorldType::SECURE_WORLD
                ? path_context_.secure_func_template_path_
                : path_context_.insecure_func_template_path_;
        for_each_file_in_path_recursive(template_path, [&](const auto& e) {
            generate_with_template(e.path(), source_context);
        });

        auto& tls_entry_func_list = WorldType == WorldType::SECURE_WORLD
                                        ? tls_secure_entry_func_list
                                        : tls_insecure_entry_func_list;
        tls_entry_func_list.insert(tls_entry_func_list.end(),
                                   tls_func_list_each_file.begin(),
                                   tls_func_list_each_file.end());
        tls_func_list_each_file.clear();
    }
};