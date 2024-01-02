cmake_minimum_required(VERSION 3.5)
project(codegen)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#set(LLVM_DIR /usr/lib/llvm-12/lib/cmake/llvm)
#set(CMAKE_PREFIX_PATH /usr/lib/llvm-12/lib/cmake/clang)
#find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(LibClang REQUIRED)
message(STATUS "Clang include directories: ${CLANG_INCLUDE_DIRS}")
message(STATUS "Clang library directories: ${CLANG_LIBRARIES}")

include_directories(${CLANG_INCLUDEDIR})
#add_definitions(${CLANG_DEFINITIONS})

set(SOURCE_FILES main.cpp parser.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${CLANG_LIBS})

