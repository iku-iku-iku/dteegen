#!/bin/bash

# 定义Docker镜像名称
IMAGE_NAME="rv-secgear"

# 启动Docker容器，并映射generated目录
docker run -v "$(pwd)/generated:/workspace/secGear/examples/generated" -w /workspace/secGear/debug -it $IMAGE_NAME /bin/bash -c "
    cmake -DCMAKE_BUILD_TYPE=Debug -DENCLAVE=PL -DSDK_PATH=/root/dev/sdk -DSSL_PATH=/root/dev/sdk/penglai_sdk_ssl .. &&
    make && 
    mkdir -p /workspace/secGear/examples/generated/build &&
    rm -rf /workspace/secGear/examples/generated/build/* &&
    cp -r /workspace/secGear/debug/examples/generated/host/* /workspace/secGear/examples/generated/build/ &&
    mv /workspace/secGear/examples/generated/enclave/penglai*ELF /workspace/secGear/examples/generated/enclave/enclave.signed.so &&
    cp /workspace/secGear/examples/generated/enclave/enclave.signed.so /workspace/secGear/examples/generated/build/
"
