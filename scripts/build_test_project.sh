#!/bin/bash
cd test_project
mkdir -p build
cd build
cmake .. && make && ./test_project
