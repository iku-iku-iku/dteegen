project(test_project_cpp)

add_executable(${PROJECT_NAME} insecure/main.cpp secure/add.cpp)
