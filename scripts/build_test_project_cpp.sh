#!/bin/bash
cd test_project_cpp
mkdir -p build
cd build
cmake .. && make && ./test_project_cpp
