FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    clang \
    libclang-dev \
    cmake \
    g++

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
