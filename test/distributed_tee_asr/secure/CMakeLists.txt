set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(secure_include)
add_subdirectory(secure_lib)

set(SOURCE_FILES
# NOTE: you can add your secure source files here
  sherpa-ncnn.cpp
)

add_library(secure ${SOURCE_FILES})

target_include_directories(secure PRIVATE ${INCLUDE_DIRS})
target_link_libraries(secure
  ${STATIC_LIBS}
)
