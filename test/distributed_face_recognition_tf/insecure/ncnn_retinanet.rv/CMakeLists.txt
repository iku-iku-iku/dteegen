
project(retinanet)
cmake_minimum_required(VERSION 3.1)
set(name ${PROJECT_NAME})
add_library(${name} ${name}.cpp)
set(CMAKE_BUILD_TYPE Debug)
target_compile_definitions(${name} PUBLIC USE_NCNN_SIMPLEOCV)
include_directories(ncnn)
target_link_libraries(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libncnn.a pthread)
