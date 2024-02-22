#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 <target>"
	exit 1
fi

CODEGEN=./build/codegen

perf record --call-graph dwarf $CODEGEN convert $1
