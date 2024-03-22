set(CLIENT_SOURCE_FILES
# NOTE: you can add your insecure source files here
  client.cpp file.cpp
)

set(SERVER_SOURCE_FILES
# NOTE: you can add your insecure source files here
  server.cpp
)

set(TEST_SOURCE_FILES
# NOTE: you can add your insecure source files here
  test.cpp file.cpp
)

set(COMPUTE_NODE_FILES
# NOTE: you can add your insecure source files here
  compute_node.cpp file.cpp
)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
  add_subdirectory(ncnn_retinanet.x64)
  include_directories(ncnn_retinanet.x64)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "riscv")
  add_subdirectory(ncnn_retinanet.rv)
  include_directories(ncnn_retinanet.rv)
endif()

add_executable(server ${SERVER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})
add_executable(test ${TEST_SOURCE_FILES})
add_executable(compute_node ${COMPUTE_NODE_FILES})

find_package(foonathan_memory REQUIRED)
target_link_libraries(server secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(client retinanet secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(test retinanet secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(compute_node secure distributed_tee fastrtps fastcdr foonathan_memory rt)
