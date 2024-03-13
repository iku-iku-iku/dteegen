path: host/secure/CMakeLists.txt
# Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
# secGear is licensed under the Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#     http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
# PURPOSE.
# See the Mulan PSL v2 for more details.
include(./function.cmake)
${host_secure_cmake}

#set auto code prefix
set(PREFIX ${project})

#set auto code
if(CC_GP)
  set(AUTO_FILES  ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_u.h ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_u.c ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_args.h)
  add_custom_command(OUTPUT ${AUTO_FILES}
    DEPENDS ${CURRENT_ROOT_PATH}/${EDL_FILE}
    COMMAND ${CODEGEN} --${CODETYPE} --untrusted ${CURRENT_ROOT_PATH}/${EDL_FILE} --search-path ${LOCAL_ROOT_PATH}/inc/host_inc/gp)
endif()

if(CC_SGX)
  set(AUTO_FILES  ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_u.h ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_u.c)
  add_custom_command(OUTPUT ${AUTO_FILES}
    DEPENDS ${CURRENT_ROOT_PATH}/${EDL_FILE}
    COMMAND ${CODEGEN} --${CODETYPE} --untrusted ${CURRENT_ROOT_PATH}/${EDL_FILE} --search-path ${LOCAL_ROOT_PATH}/inc/host_inc/sgx  --search-path ${SGXSDK}/include)
endif()

if(CC_PL)
  set(AUTO_FILES  ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_u.h ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_u.c)
  add_custom_command(OUTPUT ${AUTO_FILES}
    DEPENDS ${CURRENT_ROOT_PATH}/${EDL_FILE}
    COMMAND ${CODEGEN} --${CODETYPE} --untrusted ${CURRENT_ROOT_PATH}/${EDL_FILE} --search-path ${LOCAL_ROOT_PATH}/inc/host_inc/penglai)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS}  -s")

if(CC_GP)
  if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
    link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  endif()
  #add_executable(${OUTPUT} ${SOURCE_FILE} ${AUTO_FILES})
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  target_include_directories(${OUTPUT} PRIVATE
            ${CMAKE_BINARY_DIR}/inc
            ${LOCAL_ROOT_PATH}/inc/host_inc
            ${LOCAL_ROOT_PATH}/inc/host_inc/gp
            ${CMAKE_CURRENT_BINARY_DIR})
  endforeach()
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
    target_link_directories(${OUTPUT} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    endforeach()
  endif()
endif()

if(CC_SGX)
  if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
    link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  endif()
  #add_executable(${OUTPUT} ${SOURCE_FILE} ${AUTO_FILES})
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  target_include_directories(${OUTPUT} PRIVATE
   ${LOCAL_ROOT_PATH}/inc/host_inc
   ${LOCAL_ROOT_PATH}/inc/host_inc/sgx
   ${CMAKE_CURRENT_BINARY_DIR})
  endforeach()
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
    target_link_directories(${OUTPUT} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    endforeach()
  endif()
endif()

# _addlibrary(__z_auto_lib ${AUTO_FILES})
find_package(foonathan_memory REQUIRED)
_addlibrary(__z_auto_lib ${AUTO_FILES} ../../z_enclave_env_provider.cpp)
target_link_libraries(__z_auto_lib distributed_tee fastrtps fastcdr foonathan_memory sm2 miracl rt)
target_include_directories(__z_auto_lib PRIVATE
  ${LOCAL_ROOT_PATH}/inc/host_inc
  ${LOCAL_ROOT_PATH}/inc/host_inc/penglai
  ${CMAKE_CURRENT_BINARY_DIR})

if(CC_PL)
  if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
    link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  endif()
  #add_executable(${OUTPUT} ${SOURCE_FILE} ${AUTO_FILES})
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  target_include_directories(${OUTPUT} PRIVATE
   ${LOCAL_ROOT_PATH}/inc/host_inc
   ${LOCAL_ROOT_PATH}/inc/host_inc/penglai
   ${CMAKE_CURRENT_BINARY_DIR})
  endforeach()
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
    target_link_directories(${OUTPUT} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    endforeach()
  endif()
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  target_link_libraries(${OUTPUT} __z_auto_lib)
  endforeach()

endif()

if(CC_SIM)
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  target_link_libraries(${OUTPUT} secgearsim pthread)
  endforeach()
else()
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  target_link_libraries(${OUTPUT} secgear pthread)
  endforeach()
endif()
foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
set_target_properties(${OUTPUT} PROPERTIES SKIP_BUILD_RPATH TRUE)
endforeach()

if(CC_GP)
  #itrustee install whitelist /vender/bin/teec_hello
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  install(TARGETS  ${OUTPUT}
         RUNTIME
         DESTINATION /vendor/bin/
          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
  endforeach()
endif()

if(CC_SGX)
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  install(TARGETS  ${OUTPUT}
         RUNTIME
         DESTINATION ${CMAKE_BINARY_DIR}/bin/
          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
  endforeach()
endif()

if(CC_PL)
  foreach(OUTPUT IN LISTS TEE_LIBRARY_TARGETS)
  install(TARGETS  ${OUTPUT}
  RUNTIME
  DESTINATION ${CMAKE_BINARY_DIR}/bin/
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
  endforeach()
endif()
