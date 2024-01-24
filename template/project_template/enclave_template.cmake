path: enclave/CMakeLists.txt
# Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
# secGear is licensed under the Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#     http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
# PURPOSE.
# See the Mulan PSL v2 for more details.
set(ENCLAVE_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/enclave_lib)
if (IS_DIRECTORY ${ENCLAVE_LIB_PATH}) 

  file(GLOB CHILDREN RELATIVE ${ENCLAVE_LIB_PATH} ${ENCLAVE_LIB_PATH}/*)
  set(ENCLAVE_LIB_DIR_LIST "")
  foreach(CHILD ${CHILDREN})
      if(IS_DIRECTORY ${ENCLAVE_LIB_PATH}/${CHILD})
        list(APPEND ENCLAVE_LIB_DIR_LIST ${CHILD})
      endif()
  endforeach()

endif()

add_subdirectory(enclave_lib)
add_subdirectory(enclave_include)

# foreach(LIB ${ENCLAVE_LIB_DIR_LIST})
#   add_subdirectory(${ENCLAVE_LIB_PATH}/${LIB})
# endforeach()

#set auto code prefix
set(PREFIX ${project})

#set sign key
set(PEM Enclave_private.pem)

#set sign tool
set(SIGN_TOOL ${LOCAL_ROOT_PATH}/tools/sign_tool/sign_tool.sh)

file(GLOB_RECURSE SOURCE_FILES 
"${CMAKE_CURRENT_SOURCE_DIR}/insecure/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/insecure/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/insecure/*.cc" 
"${CMAKE_CURRENT_SOURCE_DIR}/secure/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/secure/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/secure/*.cc"
)
#set enclave src code
#set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

set(INC_DIRS "" CACHE INTERNAL "Include directories")
function(add_include_directories_recursively root_dir)
    if(IS_DIRECTORY ${root_dir})
        set(local_inc_dirs ${root_dir}) # 创建一个本地变量
        file(GLOB sub_dirs LIST_DIRECTORIES true RELATIVE ${root_dir} ${root_dir}/*)
        foreach(sub_dir IN LISTS sub_dirs)
            if(IS_DIRECTORY ${root_dir}/${sub_dir} AND NOT ${sub_dir} STREQUAL ".." AND NOT ${sub_dir} STREQUAL ".")
                add_include_directories_recursively(${root_dir}/${sub_dir})
                list(APPEND local_inc_dirs ${INC_DIRS}) # 将子调用的结果添加到本地变量
            endif()
        endforeach()
        set(INC_DIRS ${local_inc_dirs} PARENT_SCOPE) # 更新父作用域的变量
    endif()
endfunction()

add_include_directories_recursively("${CMAKE_CURRENT_SOURCE_DIR}/insecure")
add_include_directories_recursively("${CMAKE_CURRENT_SOURCE_DIR}/secure")
foreach(dir ${INCLUDE_DIRS})
  list(APPEND INC_DIRS ${dir})
endforeach()

set(COMPILER_INCLUDES "")
foreach(dir ${INC_DIRS})
    list(APPEND COMPILER_INCLUDES "-I${dir}")
    include_directories(${dir})
endforeach()
string(REPLACE ";" "\t" COMPILER_INCLUDES "${COMPILER_INCLUDES}")
message(STATUS "INC_DIRS: ${COMPILER_INCLUDES}")
# set(ENCLAVE_LIB_LINKS "")
# foreach(dir IN LISTS ENCLAVE_LIB_DIR_LIST)
#     list(APPEND ENCLAVE_LIB_LINKS "$<TARGET_FILE:${dir}>")
# endforeach()
# string(REPLACE ";" "\t" ENCLAVE_LIB_LINKS "${ENCLAVE_LIB_LINKS}")

#set log level
set(PRINT_LEVEL 0)
add_definitions(-DPRINT_LEVEL=${PRINT_LEVEL})

if(CC_GP)
  #set signed output
  set(OUTPUT ${UUID}.sec)
  #set whilelist. default: /vendor/bin/teec_hello
  set(WHITE_LIST_0 /vendor/bin/${project})
  set(WHITE_LIST_OWNER root)
  set(WHITE_LIST_1 /vendor/bin/secgear_${project})
  set(WHITELIST WHITE_LIST_0 WHITE_LIST_1)

  set(AUTO_FILES  ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.h ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.c ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_args.h)
  add_custom_command(OUTPUT ${AUTO_FILES}
    DEPENDS ${CURRENT_ROOT_PATH}/${EDL_FILE}
    COMMAND ${CODEGEN} --${CODETYPE} --trusted ${CURRENT_ROOT_PATH}/${EDL_FILE} --search-path ${LOCAL_ROOT_PATH}/inc/host_inc/gp)
endif()

if(CC_SGX)
  set(OUTPUT enclave.signed.so)
  set(AUTO_FILES  ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.h ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.c)
  add_custom_command(OUTPUT ${AUTO_FILES}
    DEPENDS ${CURRENT_ROOT_PATH}/${EDL_FILE}
    COMMAND ${CODEGEN} --${CODETYPE} --trusted ${CURRENT_ROOT_PATH}/${EDL_FILE} --search-path ${LOCAL_ROOT_PATH}/inc/host_inc/sgx --search-path ${SGXSDK}/include)
endif()

if(CC_PL)
  set(OUTPUT penglai-${project}-ELF)
  set(AUTO_FILES  ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.h ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.c ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_args.h)
  add_custom_command(OUTPUT ${AUTO_FILES}
    DEPENDS ${CURRENT_ROOT_PATH}/${EDL_FILE}
    COMMAND ${CODEGEN} --${CODETYPE} --trusted ${CURRENT_ROOT_PATH}/${EDL_FILE} --search-path ${LOCAL_ROOT_PATH}/inc/host_inc/penglai)
endif()

set(COMMON_C_FLAGS "-W -Wall -Werror  -fno-short-enums  -fno-omit-frame-pointer -fstack-protector \
 -Wstack-protector --param ssp-buffer-size=4 -frecord-gcc-switches -Wextra -nostdinc -nodefaultlibs \
 -fno-peephole -fno-peephole2 -Wno-main -Wno-error=unused-parameter \
        -Wno-error=unused-but-set-variable -Wno-error=format-truncation=")

set(COMMON_C_LINK_FLAGS "-Wl,-z,now -Wl,-z,relro -Wl,-z,noexecstack -Wl,-nostdlib -nodefaultlibs -nostartfiles")

if(CC_GP)

  set(CMAKE_C_FLAGS "${COMMON_C_FLAGS}  -march=armv8-a ")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS}  -s -fPIC")
  set(CMAKE_SHARED_LINKER_FLAGS  "${COMMON_C_LINK_FLAGS} -Wl,-s")

  set(ITRUSTEE_TEEDIR ${iTrusteeSDK}/)
  set(ITRUSTEE_LIBC ${iTrusteeSDK}/thirdparty/open_source/musl/libc)

  if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
    link_directories(${CMAKE_BINARY_DIR}/lib/)
  endif()

  add_library(${PREFIX} SHARED ${SOURCE_FILES} ${AUTO_FILES})

  target_include_directories( ${PREFIX} PRIVATE
 ${CMAKE_CURRENT_BINARY_DIR}
 ${CMAKE_BINARY_DIR}/inc
 ${LOCAL_ROOT_PATH}/inc/host_inc
 ${LOCAL_ROOT_PATH}/inc/host_inc/gp
 ${LOCAL_ROOT_PATH}/inc/enclave_inc
 ${LOCAL_ROOT_PATH}/inc/enclave_inc/gp
 ${ITRUSTEE_TEEDIR}/include/TA
 ${ITRUSTEE_TEEDIR}/include/TA/huawei_ext
 ${ITRUSTEE_LIBC}/arch/aarch64
 ${ITRUSTEE_LIBC}/
 ${ITRUSTEE_LIBC}/arch/arm/bits
 ${ITRUSTEE_LIBC}/arch/generic
 ${ITRUSTEE_LIBC}/arch/arm
 ${LOCAL_ROOT_PATH}/inc/enclave_inc/gp/itrustee)

  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    target_link_directories(${PREFIX} PRIVATE
         ${CMAKE_BINARY_DIR}/lib/)
  endif()

  foreach(WHITE_LIST ${WHITELIST})
    add_definitions(-D${WHITE_LIST}="${${WHITE_LIST}}")
  endforeach(WHITE_LIST)
  add_definitions(-DWHITE_LIST_OWNER="${WHITE_LIST_OWNER}")

  target_link_libraries(${PREFIX} -lsecgear_tee)

  #for trustzone compiling, you should connact us to get config and private_key.pem for test, so we will not sign and install binary in this example #
  #    add_custom_command(TARGET ${PREFIX}
  #	      POST_BUILD
  #	      COMMAND bash ${SIGN_TOOL} -d sign -x trustzone -i ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PREFIX}.so -c ${CMAKE_CURRENT_SOURCE_DIR}/manifest.txt -m ${CMAKE_CURRENT_SOURCE_DIR}/config_cloud.ini -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OUTPUT})

  #    install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OUTPUT}
  #        DESTINATION /data
  #        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE  WORLD_READ  WORLD_EXECUTE)

endif()

if(CC_SGX)
  set(SGX_DIR ${SGXSDK})
  set(CMAKE_C_FLAGS "${COMMON_C_FLAGS} -m64 -fvisibility=hidden")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS}  -s")
  set(LINK_LIBRARY_PATH ${SGX_DIR}/lib64)

  if(CC_SIM)
    set(Trts_Library_Name sgx_trts_sim)
    set(Service_Library_Name sgx_tservice_sim)
  else()
    set(Trts_Library_Name sgx_trts)
    set(Service_Library_Name sgx_tservice)
  endif()

  set(Crypto_Library_Name sgx_tcrypto)

  set(CMAKE_SHARED_LINKER_FLAGS  "${COMMON_C_LINK_FLAGS} -Wl,-z,defs -Wl,-pie -Bstatic -Bsymbolic -eenclave_entry \
 -Wl,--export-dynamic -Wl,--defsym,__ImageBase=0 -Wl,--gc-sections -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/Enclave.lds")

  if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
    link_directories(${LINK_LIBRARY_PATH})
  endif()

  add_library(${PREFIX}  SHARED ${SOURCE_FILES} ${AUTO_FILES})

  target_include_directories(${PREFIX} PRIVATE
     ${CMAKE_CURRENT_BINARY_DIR}
     ${SGX_DIR}/include/tlibc
     ${SGX_DIR}/include/libcxx
     ${SGX_DIR}/include
     ${LOCAL_ROOT_PATH}/inc/host_inc
     ${LOCAL_ROOT_PATH}/inc/host_inc/sgx)

  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
    target_link_directories(${PREFIX} PRIVATE
         ${LINK_LIBRARY_PATH})
  endif()

  target_link_libraries(${PREFIX}  -Wl,--whole-archive ${Trts_Library_Name} -Wl,--no-whole-archive
      -Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l${Crypto_Library_Name} -l${Service_Library_Name}   -Wl,--end-group)
  add_custom_command(TARGET ${PREFIX}
    POST_BUILD
    COMMAND umask 0177
    COMMAND openssl genrsa -3 -out ${PEM} 3072
    COMMAND bash ${SIGN_TOOL} -d sign -x sgx -i ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${PREFIX}.so -k ${PEM} -o ${OUTPUT} -c ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml)
endif()

if(NOT DEFINED CC_PL)
  set_target_properties(${PREFIX} PROPERTIES SKIP_BUILD_RPATH TRUE)
endif()

if(CC_PL)
  set(SDK_LIB_DIR ${PLSDK}/lib)
  set(SDK_INCLUDE_DIR ${SDK_LIB_DIR}/app/include)
  set(SDK_APP_LIB ${SDK_LIB_DIR}/libpenglai-enclave-eapp.a)
  set(MY_MUSL_LIB_DIR /workspace/riscv64-linux-musl/riscv64-linux-musl/lib)
  set(MUSL_LIB_DIR ${PLSDK}/musl/lib)
  set(MUSL_LIBC ${MUSL_LIB_DIR}/libc.a)
  set(MUSL_LIBCPP ${MY_MUSL_LIB_DIR}/libstdc++.a)
  set(MUSL_LIBATOMIC ${MY_MUSL_LIB_DIR}/libatomic.a)
  set(CC gcc)
  set(CXX g++)
  # set(CMAKE_C_COMPILER gcc)
  # set(CMAKE_CXX_COMPILER g++)
  # set(GCC_LIB /workspace/riscv64-linux-musl/lib/gcc/riscv64-linux-musl/9.4.0/libgcc.a)
  set(GCC_LIB ${SDK_LIB_DIR}/libgcc.a)
  set(SECGEAR_TEE_LIB ${CMAKE_BINARY_DIR}/lib/libsecgear_tee.a)

  set(SOURCE_C_OBJS "")

  # include_directories(${SDK_INCLUDE_DIR})
  # include_directories(${CMAKE_CURRENT_BINARY_DIR})
  # include_directories(${CMAKE_BINARY_DIR}/inc)
  # include_directories(${LOCAL_ROOT_PATH}/inc/host_inc)
  # include_directories(${LOCAL_ROOT_PATH}/inc/host_inc/penglai)
  # include_directories(${LOCAL_ROOT_PATH}/inc/enclave_inc)
  # include_directories(${LOCAL_ROOT_PATH}/inc/enclave_inc/penglai)
  #
  # # add_library(source_c_object ${AUTO_FILES})
  # set_source_files_properties(${AUTO_FILES} PROPERTIES LANGUAGE C)
  # # target_compile_options(source_c_object PRIVATE -static -Wall)
  # # target_link_libraries(source_c_object
  # #     ${SDK_APP_LIB}
  # #     ${SECGEAR_TEE_LIB}
  # #     ${GCC_LIB}
  # #     ${MUSL_LIBC}
  # # )
  # add_custom_target(generate_auto_files
  #     DEPENDS ${AUTO_FILES}
  # )
  #
  # add_library(source_objects OBJECT ${SOURCE_FILES})
  # add_dependencies(source_objects generate_auto_files)
  #
  # target_compile_options(source_objects PRIVATE -static -fno-use-cxa-atexit -Wall)
  #
  # add_executable(${OUTPUT} $<TARGET_OBJECTS:source_objects> ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.c)
  #
  #
  # # foreach(SOURCE_FILE_CPP ${SOURCE_FILES})
  # #   set_source_files_properties(${SOURCE_FILE_CPP} PROPERTIES LANGUAGE CXX)
  # # endforeach()
  # # set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.c PROPERTIES LANGUAGE C)
  # # 设置链接库
  # target_link_directories(${OUTPUT}
  #     PRIVATE
  #     ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} 
  #     ${SDK_LIB_DIR} 
  #     ${MUSL_LIB_DIR} 
  #     /usr/lib64 
  # )
  # target_link_libraries(${OUTPUT} 
  #     penglai-enclave-eapp 
  #     ${SDK_APP_LIB}
  #     ${SECGEAR_TEE_LIB}
  #     ${MUSL_LIBCPP}
  #     ${GCC_LIB}
  #     ${MUSL_LIBC}
  # )
  #
  # # 链接器选项
  # target_link_options(${OUTPUT} PRIVATE 
  #     -static 
  #     -T ${PLSDK}/app.lds
  # )

  foreach(SOURCE_FILE ${SOURCE_FILES})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" SOURCE_FILE_NAME ${SOURCE_FILE})
    set(SOURCE_OBJ ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE_NAME}.o)
    add_custom_command(
            OUTPUT ${SOURCE_OBJ}
            DEPENDS ${SOURCE_FILES}
            COMMAND ${CXX} -std=c++17 -static -Wall -D__TEE=1 ${COMPILER_INCLUDES} -I${SDK_INCLUDE_DIR} -I${CMAKE_CURRENT_BINARY_DIR} -I${CMAKE_BINARY_DIR}/inc
                -I${LOCAL_ROOT_PATH}/inc/host_inc -I${LOCAL_ROOT_PATH}/inc/host_inc/penglai -I${LOCAL_ROOT_PATH}/inc/enclave_inc
                -I${LOCAL_ROOT_PATH}/inc/enclave_inc/penglai -c -o ${SOURCE_OBJ} ${SOURCE_FILE}
            COMMENT "generate SOURCE_OBJ"
        )
    list(APPEND SOURCE_C_OBJS ${SOURCE_OBJ})
  endforeach()

  set(APP_C_OBJ ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.o)
  add_custom_command(
        OUTPUT ${APP_C_OBJ}
        DEPENDS ${AUTO_FILES}
        COMMAND ${CC} -static -Wall ${COMPILER_INCLUDES} -I${SDK_INCLUDE_DIR} -I${CMAKE_CURRENT_BINARY_DIR} -I${CMAKE_BINARY_DIR}/inc
            -I${LOCAL_ROOT_PATH}/inc/host_inc -I${LOCAL_ROOT_PATH}/inc/host_inc/penglai -I${LOCAL_ROOT_PATH}/inc/enclave_inc
            -I${LOCAL_ROOT_PATH}/inc/enclave_inc/penglai -c -o ${APP_C_OBJ} ${CMAKE_CURRENT_BINARY_DIR}/${PREFIX}_t.c
        COMMENT "generate APP_C_OBJ"
    )

  add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT}
        DEPENDS ${APP_C_OBJ} ${SOURCE_C_OBJS} ${SDK_APP_LIB} ${MUSL_LIBC} ${GCC_LIB}
        COMMAND ld -static -L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -L${SDK_LIB_DIR} -L${MUSL_LIB_DIR} -L/usr/lib64 -lpenglai-enclave-eapp -lsecgear_tee -lc -lpthread
            -o ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT} ${APP_C_OBJ} ${SOURCE_C_OBJS} ${SDK_APP_LIB} ${SECGEAR_TEE_LIB} ${STATIC_LIBS} ${MUSL_LIBCPP}
             /usr/lib/libunwind.a ${MUSL_LIBC} ${GCC_LIB} ${MUSL_LIBATOMIC} /usr/lib/libjustworkaround.a -T ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.lds
        COMMAND chmod -x ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT}
        COMMENT "generate penglai-ELF"
    )
  add_custom_target(
        ${OUTPUT} ALL
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${OUTPUT}
        COMMENT "makefile target penglai-${project}-ELF"
    )
  # add_custom_command(TARGET ${OUTPUT}
  #   POST_BUILD
  #   COMMAND chmod -x $<TARGET_FILE:${OUTPUT}>
  #   COMMENT "Setting executable permissions for ${OUTPUT}"
  # )

endif()
