cmake_minimum_required(VERSION 3.16)
# NOTE: you can change the project name
project(test_project_cpp C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(secure)
add_subdirectory(insecure)
