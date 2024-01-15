set(SOURCE_FILES
# NOTE: you can add your insecure source files here
  main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} secure)
