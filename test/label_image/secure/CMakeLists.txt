project(secure)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(secure_include)
add_subdirectory(secure_lib)
add_library(${PROJECT_NAME} label_image.cc)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
  ${STATIC_LIBS}
)
