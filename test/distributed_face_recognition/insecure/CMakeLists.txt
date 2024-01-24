set(CLIENT_SOURCE_FILES
# NOTE: you can add your insecure source files here
  client.cpp
  img_receiver.cpp
)

set(SERVER_SOURCE_FILES
# NOTE: you can add your insecure source files here
  server.cpp
  img_receiver.cpp
)

add_executable(server ${SERVER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})

find_package(foonathan_memory REQUIRED)
target_link_libraries(server secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(client secure distributed_tee fastrtps fastcdr foonathan_memory rt)
