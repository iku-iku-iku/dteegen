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
IMAGE_NAME="dteegen"

# 启动Docker容器，并映射generated目录
docker run -v "$TARGET:/workspace/secGear/examples/generated" \
	-v "$TARGET/build:/workspace/secGear/debug" \
	-w /workspace/secGear/debug -it $IMAGE_NAME /bin/bash -c "
    export C_INCLUDE_PATH=/usr/local/include/:$C_INCLUDE_PATH
    export CPLUS_INCLUDE_PATH=/usr/local/include/:$CPLUS_INCLUDE_PATH
    export CC=riscv64-linux-gnu-gcc &&
    export CXX=riscv64-linux-gnu-g++ &&
    export PATH=/root/.opam/4.12.0/bin:$PATH:/workspace/secGear/debug
    cmake -DCMAKE_BUILD_TYPE=Debug -DENCLAVE=PL -DSDK_PATH=/root/dev/sdk -DSSL_PATH=/root/dev/sdk/penglai_sdk_ssl .. &&
    make -j8 && 
    cp -r /workspace/secGear/debug/examples/generated/host/insecure/* /workspace/secGear/examples/generated/build/ &&
    mv /workspace/secGear/examples/generated/enclave/penglai*ELF /workspace/secGear/examples/generated/enclave/enclave.signed.so &&
    penglai_sign sign -enclave /workspace/secGear/examples/generated/enclave/enclave.signed.so -key /root/SM2PrivateKey.pem -out /workspace/secGear/examples/generated/build/enclave.signed.so
"
