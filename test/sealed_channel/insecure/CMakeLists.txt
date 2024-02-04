add_executable(sealed_channel main.cpp receiver.cpp)

find_package(foonathan_memory REQUIRED)
target_link_libraries(sealed_channel secure distributed_tee fastrtps fastcdr foonathan_memory rt)
