cmake_minimum_required(VERSION 3.5)
project(dteegen)

# use C++17
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(LLVM_DIR /usr/lib/llvm-12/lib/cmake/llvm)
#set(CMAKE_PREFIX_PATH /usr/lib/llvm-12/lib/cmake/clang)
#find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(LibClang REQUIRED)
message(STATUS "Clang include directories: ${CLANG_INCLUDE_DIRS}")
message(STATUS "Clang library directories: ${CLANG_LIBRARIES}")

include_directories(${CLANG_INCLUDEDIR} src)
#add_definitions(${CLANG_DEFINITIONS})

set(SOURCE_FILES src/main.cpp src/parser.cpp src/collector.cpp src/elf_parser.cpp src/template.cpp src/pipe/cmake_transform.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${CLANG_LIBS} pthread)

# if debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -gdwarf-4)
endif()
