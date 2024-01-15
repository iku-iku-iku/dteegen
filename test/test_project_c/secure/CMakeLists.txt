set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(secure_include)
add_subdirectory(secure_lib)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.c)

add_library(secure ${SOURCES})

target_include_directories(secure PRIVATE ${INCLUDE_DIRS})
target_link_libraries(secure
  ${STATIC_LIBS}
)
