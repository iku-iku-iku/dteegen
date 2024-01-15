# NOTE: you can change the project name
add_executable(${PROJECT_NAME} main.c strcat.c)

target_link_libraries(${PROJECT_NAME} secure)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../secure/secure_include)
