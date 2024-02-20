# Install script for directory: /media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/libncnn.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ncnn" TYPE FILE FILES
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/allocator.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/benchmark.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/blob.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/c_api.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/command.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/cpu.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/datareader.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/gpu.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/layer.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/layer_shader_type.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/layer_type.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/mat.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/modelbin.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/net.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/option.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/paramdict.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/pipeline.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/pipelinecache.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/simpleocv.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/simpleomp.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/simplestl.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/simplemath.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/simplevk.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/src/vulkan_header_fix.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/ncnn_export.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/layer_shader_type_enum.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/layer_type_enum.h"
    "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/platform.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ncnn/ncnn.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ncnn/ncnn.cmake"
         "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/CMakeFiles/Export/790e04ecad7490f293fc4a38f0c73eb1/ncnn.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ncnn/ncnn-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ncnn/ncnn.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ncnn" TYPE FILE FILES "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/CMakeFiles/Export/790e04ecad7490f293fc4a38f0c73eb1/ncnn.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ncnn" TYPE FILE FILES "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/CMakeFiles/Export/790e04ecad7490f293fc4a38f0c73eb1/ncnn-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ncnn" TYPE FILE FILES "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/ncnnConfig.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/media/code4love/d30f2d95-206e-43a6-94b8-40bd7e385fa3/home/code4love/PROGRAM/MyCODE/ncnn/build-host-gcc-linux/src/ncnn.pc")
endif()

