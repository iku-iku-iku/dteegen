#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 <target>"
	exit 1
fi

if [ ! -d $1 ]; then
	echo "Target $1 does not exist"
	exit 1
fi

TARGET=$(realpath $1)

mkdir -p $TARGET/build

# 定义Docker镜像名称
IMAGE_NAME="rv-secgear"

# 启动Docker容器，并映射generated目录
docker run -v "$TARGET:/workspace/secGear/examples/generated" \
	-v "$TARGET/build:/workspace/secGear/debug" \
	-w /workspace/secGear/debug -it $IMAGE_NAME /bin/bash -c "
    cmake -DCMAKE_BUILD_TYPE=Debug -DENCLAVE=PL -DSDK_PATH=/root/dev/sdk -DSSL_PATH=/root/dev/sdk/penglai_sdk_ssl .. &&
    make && 
    cp -r /workspace/secGear/debug/examples/generated/host/insecure/* /workspace/secGear/examples/generated/build/ &&
    mv /workspace/secGear/examples/generated/enclave/penglai*ELF /workspace/secGear/examples/generated/enclave/enclave.signed.so &&
    cp /workspace/secGear/examples/generated/enclave/enclave.signed.so /workspace/secGear/examples/generated/build/
"
