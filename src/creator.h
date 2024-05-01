#pragma once

#include <string>

void create(const std::string& project_path);

struct ProjectCreator {
    const std::string project_path_;

    ProjectCreator(const std::string project_path)
        : project_path_(project_path)
    {
    }

    void operator()() const
    {
        create(project_path_);
    }
};