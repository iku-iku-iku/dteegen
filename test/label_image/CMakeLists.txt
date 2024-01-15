#
# Copyright 2020 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# Builds the label_image Tensorflow Lite example.

cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(secure)
add_subdirectory(insecure)

# project(label_image_demo C CXX)
# add_subdirectory(secure_lib)
# add_subdirectory(secure_include)
#
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(TENSORFLOW_SOURCE_DIR "" CACHE PATH
#   "Directory that contains the TensorFlow project"
# )
# if(NOT TENSORFLOW_SOURCE_DIR)
#   get_filename_component(TENSORFLOW_SOURCE_DIR
#     "${CMAKE_CURRENT_LIST_DIR}/secure_lib/tensorflow-lite"
#     ABSOLUTE
#   )
# endif()

#add_subdirectory("${TENSORFLOW_SOURCE_DIR}")
# add_subdirectory(
#   "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"
#   "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
#   EXCLUDE_FROM_ALL
# )

# add_executable(${PROJECT_NAME}
#   insecure/main.cc secure/label_image.cc
# )
# target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
# message(STATUS "LIBS: ${STATIC_LIBS}")
# target_link_libraries(${PROJECT_NAME}
#   ${STATIC_LIBS}
# )
