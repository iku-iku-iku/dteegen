
set(CLIENT_SOURCE_FILES
# NOTE: you can add your insecure source files here
  client.cpp
)

set(SERVER_SOURCE_FILES
# NOTE: you can add your insecure source files here
  server.cpp
)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
add_subdirectory(insecure_lib)

add_executable(server ${SERVER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lua)

find_package(foonathan_memory REQUIRED)
target_link_libraries(server ${STATIC_LIBS} secure distributed_tee fastrtps fastcdr foonathan_memory rt)
target_link_libraries(client ${STATIC_LIBS} secure distributed_tee fastrtps fastcdr foonathan_memory rt)
