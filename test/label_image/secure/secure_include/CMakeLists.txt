set(INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/lite"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/eigen"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/absl"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/gemmlowp"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/ruy"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/neon_2_sse"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/farmhash/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/flatbuffers/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/fp16/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/cpuinfo"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/cpuinfo/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/cpuinfo/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/tensorflow-2.6.5/downloads/cpuinfo/deps/clog/include"
)

set(INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)
