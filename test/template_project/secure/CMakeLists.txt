set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(secure_include)
add_subdirectory(secure_lib)

add_library(secure get_string.cpp)

target_include_directories(secure PRIVATE ${INCLUDE_DIRS})
target_link_libraries(secure
  ${STATIC_LIBS}
)
