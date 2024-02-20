#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 <output-dir>"
	exit 1
fi

if [ ! -d $1 ]; then
	echo "Error: $1 is not a directory"
	exit 1
fi

PROJECT_DIR=$(realpath $1)
PROJECT_NAME=$(basename $PROJECT_DIR)
PROJECT_PARENT=$(dirname $PROJECT_DIR)
echo "Generating for project $PROJECT_PARENT/$PROJECT_NAME"

TARGET="$1.generated"
mkdir -p $TARGET

# 定义Docker镜像名称
IMAGE_NAME="rv-secgear"

# 启动Docker容器，并映射generated目录
docker run -v "$PROJECT_DIR:/workspace/dteegen/$PROJECT_NAME" \
	-v "$TARGET:/workspace/secGear/examples/generated" \
	-v "$TARGET/build:/workspace/secGear/debug" \
	-v /usr/bin/qemu-riscv64-static:/usr/bin/qemu-riscv64-static \
	-w /workspace/dteegen -it $IMAGE_NAME /bin/bash -c "
    export PATH=/root/.opam/4.12.0/bin:\$PATH:/workspace/secGear/debug &&
    echo 'dteegen $PROJECT_NAME' &&
    dteegen $PROJECT_NAME &&
    mkdir -p /workspace/secGear/examples/generated && 
    cp -r generated/* /workspace/secGear/examples/generated &&
    cd /workspace/secGear/debug &&
    cmake -DCMAKE_BUILD_TYPE=Debug -DENCLAVE=PL -DSDK_PATH=/root/dev/sdk -DSSL_PATH=/root/dev/sdk/penglai_sdk_ssl .. &&
    make -j8 &&
    cp -r /workspace/secGear/debug/examples/generated/host/insecure/* /workspace/secGear/examples/generated/build/ &&
    mv /workspace/secGear/examples/generated/enclave/penglai*ELF /workspace/secGear/examples/generated/enclave/enclave.signed.so &&
    cp /workspace/secGear/examples/generated/enclave/enclave.signed.so /workspace/secGear/examples/generated/build/
"
