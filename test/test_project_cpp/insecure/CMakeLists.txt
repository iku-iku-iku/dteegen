add_executable(${PROJECT_NAME} main.cpp num.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

target_link_libraries(${PROJECT_NAME} secure distributed_tee sm2 miracl)
