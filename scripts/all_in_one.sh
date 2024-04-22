#!/bin/bash
if [ $# -lt 2 ]; then
	echo "Usage: $0 <command> <target> [optional]"
	exit 1
fi

# 定义Docker镜像名称
IMAGE_NAME="registry.cn-hangzhou.aliyuncs.com/dteegen/dteegen:1.0.2"

USER=$(id -u):$(id -g)

DIR=$2

case $1 in
    inject)
        if [ ! -d $2 ]; then
            echo "$2 is not a directory"
            exit 1
        fi
        if [ ! -d $3 ]; then
            echo "$3 is not a directory"
            exit 1
        fi
        MOUNT_PATH=$2
        PROJECT_PATH=$3
        docker run -v "$MOUNT_PATH:/tmp/mount" --rm -it $IMAGE_NAME /bin/bash -c "
        cp -r /usr/riscv64-linux-gnu/lib/* /tmp/mount/system/lib && 
        cp /usr/riscv64-linux-gnu/lib/ld-linux-riscv64-lp64d.so.1 /tmp/mount/system/lib
        "
        sudo cp "$PROJECT_PATH".generated/build/lib/penglai/libpenglai_0.so "$MOUNT_PATH"/system/lib64
        ;;
    remove)
        if [ -d $2 ]; then
            rm -rf $2 $2.generated
        fi
        ;;
    create)
        if [ -d $2 ]; then
            echo "$2 already exists!"
            exit 1
        fi

docker run -v "$(pwd):/tmp/gen_target" \
	-w /workspace/dteegen --rm -it $IMAGE_NAME /bin/bash -c "dteegen create $DIR && chown -R $USER $DIR && mv $DIR /tmp/gen_target"
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
	-w /workspace/dteegen -it $IMAGE_NAME /bin/bash -c "
    export C_INCLUDE_PATH=/usr/local/include/:$C_INCLUDE_PATH
    export CPLUS_INCLUDE_PATH=/usr/local/include/:$CPLUS_INCLUDE_PATH
    export CC=riscv64-linux-gnu-gcc &&
    export CXX=riscv64-linux-gnu-g++ &&
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
    penglai_sign sign -enclave /workspace/secGear/examples/generated/enclave/enclave.signed.so -key /root/SM2PrivateKey.pem -out /workspace/secGear/examples/generated/build/enclave.signed.so
"
        ;;

esac
