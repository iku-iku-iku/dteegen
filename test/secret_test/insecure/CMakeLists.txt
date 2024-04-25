
set(CLIENT_SOURCE_FILES
# NOTE: you can add your insecure source files here
  client.cpp
)

add_executable(client ${CLIENT_SOURCE_FILES})

target_link_libraries(client secure)


set(SERVER_SOURCE_FILES
# NOTE: you can add your insecure source files here
  server.cpp
)
add_executable(server ${SERVER_SOURCE_FILES})
target_link_libraries(server secure)