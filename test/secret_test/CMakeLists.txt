
cmake_minimum_required(VERSION 3.16)
project(secret_test C CXX)
include_directories(.dev)
include_directories(.dev/TEE-Capability)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(secure)
add_subdirectory(insecure)
