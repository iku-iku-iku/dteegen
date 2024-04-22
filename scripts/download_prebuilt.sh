#!/bin/bash

mkdir -p polyos && cd polyos
if [ ! -d "images" ]; then
    wget https://filebin.net/m6q51s538bgd12p1/images.tar.gz &&
    tar -zxvf images.tar.gz &&
    mkdir -p out/riscv64_virt/packages/phone/images &&
    mv images/* out/riscv64_virt/packages/phone/images
fi
