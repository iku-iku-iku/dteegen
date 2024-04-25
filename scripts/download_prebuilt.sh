#!/bin/bash

mkdir -p polyos && cd polyos
if [ ! -d "images" ]; then
    wget -O images.tar.gz https://ipads.se.sjtu.edu.cn:1313/f/aaf69826370c4f7eaa30/?dl=1 &&
    tar -zxvf images.tar.gz &&
    mkdir -p out/riscv64_virt/packages/phone/images &&
    mv images/* out/riscv64_virt/packages/phone/images
fi
