project(label_image_demo C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} secure)
