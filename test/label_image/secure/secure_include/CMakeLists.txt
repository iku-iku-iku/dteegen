set(INCLUDE_DIRS
  #"${CMAKE_CURRENT_SOURCE_DIR}"
  #"${CMAKE_CURRENT_SOURCE_DIR}/tensorflow/lite"
  "${CMAKE_CURRENT_SOURCE_DIR}/Tensorflow-lite-Demo"
  "${CMAKE_CURRENT_SOURCE_DIR}/Tensorflow-lite-Demo/tensorflow"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/eigen"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/absl"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/gemmlowp"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/ruy"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/neon_2_sse"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/farmhash/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/flatbuffers/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/fp16/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/cpuinfo"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/cpuinfo/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/cpuinfo/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/downloads/cpuinfo/deps/clog/include"
)

set(INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)
