#!/bin/bash

mkdir -p build
cd build
# check debug needed
if [ "x$1" = "xdebug" ]; then
	cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug .. && ninja
else
	cmake -G Ninja .. && ninja
fi
