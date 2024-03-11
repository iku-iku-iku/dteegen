set(MAIN_SOURCE_FILES
  main.cpp
)
add_executable(main ${MAIN_SOURCE_FILES})

target_link_libraries(main secure)
