FROM riscv64/ubuntu:20.04

WORKDIR /workspace
ARG DEBIAN_FRONTEND=noninteractive
# 安装基本工具
RUN apt-get update && apt-get install -y \
strace \
    build-essential \
    cmake \
    llvm \
    clang \
    libclang-dev \
    # ocaml \
    # ocaml-dune \
    git \
    unzip \
    wget \
    vim \
    curl

COPY libjustworkaround.a /usr/lib
COPY libunwind.a /usr/lib

COPY build/dteegen /usr/bin/dteegen
COPY codegen /usr/bin
RUN mkdir -p /workspace/dteegen
COPY template /workspace/dteegen/template

# WORKDIR /root
# COPY opam.tar.gz /root
# RUN tar -zxvf opam.tar.gz && rm opam.tar.gz

WORKDIR /workspace
COPY riscv64-linux-musl.tar.gz /workspace
RUN tar -zxvf riscv64-linux-musl.tar.gz && rm riscv64-linux-musl.tar.gz

COPY secGear.tar.gz /workspace
RUN tar -zxvf secGear.tar.gz && rm secGear.tar.gz

COPY sdk.zip /workspace
RUN unzip sdk.zip && mkdir -p /root/dev && mv sdk /root/dev/sdk && rm sdk.zip

WORKDIR /root
COPY foonathan_memory_vendor.tar.gz /root
RUN tar -zxvf foonathan_memory_vendor.tar.gz && rm foonathan_memory_vendor.tar.gz
WORKDIR foonathan_memory_vendor/build
RUN make install && rm -rf /root/foonathan_memory_vendor

WORKDIR /root
COPY confidential-distributed-softbus.tar.gz /root
RUN tar -zxvf confidential-distributed-softbus.tar.gz && rm confidential-distributed-softbus.tar.gz
WORKDIR confidential-distributed-softbus/build
RUN make install && cd ../.. && rm -rf /root/confidential-distributed-softbus

WORKDIR /workspace/build_deps
COPY TEE-Capability.tar.gz /workspace/build_deps
RUN tar -zxvf TEE-Capability.tar.gz && rm TEE-Capability.tar.gz
WORKDIR TEE-Capability/build
RUN make install && cd ../.. && rm -rf /workspace/build_deps/TEE-Capability

WORKDIR /workspace

RUN mkdir -p /workspace/secGear/debug 

WORKDIR /workspace/secGear/debug 
ENV PATH="/workspace/riscv64-linux-musl/bin:/workspace/secGear/bin:${PATH}"

ENV http_proxy "http://172.17.0.1:7890"  
ENV HTTP_PROXY "http://172.17.0.1:7890"  
ENV https_proxy "http://172.17.0.1:7890"  
ENV HTTPS_PROXY "http://172.17.0.1:7890"  

CMD ["/bin/bash"]  

