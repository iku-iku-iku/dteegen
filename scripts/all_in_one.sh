#!/bin/bash
if [ $# -ne 2 ]; then
	echo "Usage: $0 <command> <target>"
	exit 1
fi

# 定义Docker镜像名称
IMAGE_NAME="registry.cn-hangzhou.aliyuncs.com/dteegen/dteegen:1.0.0"

USER=$(id -u):$(id -g)

gen_target() {
    docker run -v "$(pwd):/workspace/dteegen" \
        -v /usr/bin/qemu-riscv64-static:/usr/bin/qemu-riscv64-static \
        -w /workspace/dteegen --rm -it $IMAGE_NAME /bin/bash -c "dteegen create $2 && chown -R $USER $2"
}

case $1 in
    remove)
        if [ -d $2 ]; then
            rm -rf $2
        fi
        ;;
    create)
        if [ -d $2 ]; then
            echo "$2 already exists!"
            exit 1
        fi

docker run -v "$(pwd):/workspace/dteegen" \
	-v /usr/bin/qemu-riscv64-static:/usr/bin/qemu-riscv64-static \
	-w /workspace/dteegen --rm -it $IMAGE_NAME /bin/bash -c "dteegen create $2 && chown -R $USER $2"
        ;;
    build)
        if [ ! -d $2 ]; then
            echo "$2 is not a directory"
            exit 1
        fi
        cd $2 && mkdir -p build && cd build && cmake .. && make
        ;;
    deploy)
        if [ ! -d $2 ]; then
            echo "$2 is not a directory"
            exit 1
        fi

PROJECT_DIR=$(realpath $2)
PROJECT_NAME=$(basename $PROJECT_DIR)
PROJECT_PARENT=$(dirname $PROJECT_DIR)
echo "Generating for project $PROJECT_PARENT/$PROJECT_NAME"

TARGET="$2.generated"
mkdir -p $TARGET
TARGET=$(realpath $TARGET)

docker run -v "$PROJECT_DIR:/workspace/dteegen/$PROJECT_NAME" \
	-v "$TARGET:/workspace/secGear/examples/generated" \
	-v "$TARGET/build:/workspace/secGear/debug" \
	-v /usr/bin/qemu-riscv64-static:/usr/bin/qemu-riscv64-static \
	-w /workspace/dteegen -it $IMAGE_NAME /bin/bash -c "
    export PATH=/root/.opam/4.12.0/bin:\$PATH:/workspace/secGear/debug &&
    echo 'dteegen convert $PROJECT_NAME' &&
    dteegen convert $PROJECT_NAME &&
    mkdir -p /workspace/secGear/examples/generated && 
    cp -r generated/* /workspace/secGear/examples/generated &&
    cd /workspace/secGear/debug &&
    cmake -DCMAKE_BUILD_TYPE=Debug -DENCLAVE=PL -DSDK_PATH=/root/dev/sdk -DSSL_PATH=/root/dev/sdk/penglai_sdk_ssl .. &&
    make -j8 &&
    cp -r /workspace/secGear/debug/examples/generated/host/insecure/* /workspace/secGear/examples/generated/build/ &&
    mv /workspace/secGear/examples/generated/enclave/penglai*ELF /workspace/secGear/examples/generated/enclave/enclave.signed.so &&
    cp /workspace/secGear/examples/generated/enclave/enclave.signed.so /workspace/secGear/examples/generated/build/
"
        ;;

esac