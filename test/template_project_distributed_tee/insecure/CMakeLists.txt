#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

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

target_link_libraries(server secure)
target_link_libraries(client secure)
target_link_libraries(compute_node secure)
