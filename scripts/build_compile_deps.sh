#!/bin/bash
# DOCKERFILE=Dockerfile.build_deps
# if [ ! -f "$DOCKERFILE" ]; then
# 	echo "Dockerfile not found"
# 	exit 1
# fi
#IMAGE=rv64-build_env

#docker build -t $IMAGE -f $DOCKERFILE .
# docker run -v /usr/bin/qemu-riscv64-static:/usr/bin/qemu-riscv64-static \
# 	-v $(pwd):/workspace \
# 	-w /workspace \
# 	-it $IMAGE \
# 	/bin/bash -c '
# mkdir -p build_deps
# cp -r TEE-Capability build_deps/
# cd build_deps/TEE-Capability
# rm -rf build
# mkdir -p build
# cd build
# cmake .. && make
# '
IMAGE_NAME="dteegen"

# 启动Docker容器，并映射generated目录
docker run -v $(pwd):/workspace \
 	-w /workspace \
	-it $IMAGE_NAME /bin/bash -c "
    export C_INCLUDE_PATH=/usr/local/include/:$C_INCLUDE_PATH
    export CPLUS_INCLUDE_PATH=/usr/local/include/:$CPLUS_INCLUDE_PATH
    export CC=riscv64-linux-gnu-gcc &&
    export CXX=riscv64-linux-gnu-g++ &&
	mkdir -p build_deps
	cp -r TEE-Capability build_deps/
	cd build_deps/TEE-Capability
	rm -rf build
	mkdir -p build
	cd build
	cmake .. && make
"
