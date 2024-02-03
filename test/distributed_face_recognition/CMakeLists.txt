cmake_minimum_required(VERSION 3.16)
# NOTE: you can change the project name
project(helloworld C CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(secure)
add_subdirectory(insecure)
