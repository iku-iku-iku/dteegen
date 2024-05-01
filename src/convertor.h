#pragma once

#include <filesystem>

void generate_secgear(const std::filesystem::path project_root);

struct SecGearConvertor
{
    const std::filesystem::path project_root_;

    SecGearConvertor(const std::filesystem::path project_root)
        : project_root_(project_root)
    {
    }

    void operator()() const
    {
        generate_secgear(project_root_);
    }
};