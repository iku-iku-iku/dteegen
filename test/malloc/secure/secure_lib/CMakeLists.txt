
# NOTE: please your libs in the riscv64 and x64 directories

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
  set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/x64")
  message(STATUS "Target architecture is x64, LIB DIR is ${LIB_DIR}")
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "riscv")
  set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/riscv64")
  message(STATUS "Target architecture is RISC-V")
else()
  message(FATAL_ERROR "Target architecture is not supported")
endif()

file(GLOB_RECURSE STATIC_LIBS "${LIB_DIR}/*.a")
set(STATIC_LIBS ${STATIC_LIBS} PARENT_SCOPE)
message(STATUS "Static libs: ${STATIC_LIBS}")
