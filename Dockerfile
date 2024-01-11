FROM riscv64/ubuntu:20.04

WORKDIR /workspace
ARG DEBIAN_FRONTEND=noninteractive
# 安装基本工具
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    opam \
    ocaml-dune \
    vim \
    curl

COPY secGear.tar.gz /workspace
RUN tar -zxvf secGear.tar.gz && rm secGear.tar.gz

COPY sdk.zip /workspace
RUN unzip sdk.zip && mkdir -p /root/dev && mv sdk /root/dev/sdk && rm sdk.zip

RUN mkdir -p /workspace/secGear/debug 

ENV PATH="/workspace/secGear/bin:${PATH}"

CMD ["/bin/bash"]

