set(CLIENT_SOURCE_FILES
# NOTE: you can add your insecure source files here
  client.cpp
)

set(SERVER_SOURCE_FILES
# NOTE: you can add your insecure source files here
  server.cpp
)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(COMPUTE_NODE_SOURCE_FILES
# NOTE: you can add your insecure source files here
  compute_node.cpp
)

add_executable(server ${SERVER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})
add_executable(compute_node ${COMPUTE_NODE_SOURCE_FILES})

find_package(foonathan_memory REQUIRED)
target_link_libraries(server secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(client secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(compute_node secure distributed_tee fastrtps fastcdr foonathan_memory rt)
