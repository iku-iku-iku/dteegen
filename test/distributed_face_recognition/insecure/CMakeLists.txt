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

set(TEST_SOURCE_FILES
# NOTE: you can add your insecure source files here
  test.cpp
  img_receiver.cpp
)

add_subdirectory(ncnn_retinanet)

add_executable(server ${SERVER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})
add_executable(test ${TEST_SOURCE_FILES})

find_package(foonathan_memory REQUIRED)
target_link_libraries(server secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(client retinanet secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(test retinanet secure distributed_tee fastrtps fastcdr foonathan_memory rt)
