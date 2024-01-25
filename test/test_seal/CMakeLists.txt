cmake_minimum_required(VERSION 3.16)
# NOTE: you can change the project name
project(test_seal C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
add_subdirectory(secure)
add_subdirectory(insecure)
