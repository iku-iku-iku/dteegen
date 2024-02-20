#!/bin/bash

mkdir -p build
cd build

# check ninja exists
if [ ! -x "$(command -v ninja)" ]; then
	echo "ninja is not installed"
	# check debug needed
	if [ "x$1" = "xdebug" ]; then
		cmake -DCMAKE_BUILD_TYPE=Debug .. && make
	else
		cmake .. && make
	fi
else
	# check debug needed
	if [ "x$1" = "xdebug" ]; then
		cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug .. && ninja
	else
		cmake -G Ninja .. && ninja
	fi
fi
