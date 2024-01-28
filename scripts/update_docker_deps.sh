#!/bin/bash

TEE_SDK=TEE-Capability

if [ -d "build_deps/$TEE_SDK" ]; then
	rm $TEE_SDK.tar.gz
	cd build_deps
	tar -czvf $TEE_SDK.tar.gz $TEE_SDK
	cd ..
	mv build_deps/$TEE_SDK.tar.gz .
fi

if [ -d "secGear" ]; then
	rm secGear.tar.gz
	tar -czvf secGear.tar.gz secGear
fi
