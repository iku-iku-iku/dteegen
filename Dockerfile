FROM riscv64/ubuntu:20.04

WORKDIR /workspace
ARG DEBIAN_FRONTEND=noninteractive
# 安装基本工具
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    unzip \
    wget \
    vim \
    curl

COPY codegen /usr/bin
COPY riscv64-linux-musl.tar.gz /workspace
RUN tar -zxvf riscv64-linux-musl.tar.gz && rm riscv64-linux-musl.tar.gz

COPY secGear.tar.gz /workspace
RUN tar -zxvf secGear.tar.gz && rm secGear.tar.gz

COPY sdk.zip /workspace
RUN unzip sdk.zip && mkdir -p /root/dev && mv sdk /root/dev/sdk && rm sdk.zip

RUN mkdir -p /workspace/secGear/debug 

ENV PATH="/workspace/riscv64-linux-musl/bin:/workspace/secGear/bin:${PATH}"

ENV http_proxy "http://172.17.0.1:7890"  
ENV HTTP_PROXY "http://172.17.0.1:7890"  
ENV https_proxy "http://172.17.0.1:7890"  
ENV HTTPS_PROXY "http://172.17.0.1:7890"  

CMD ["/bin/bash"]  

