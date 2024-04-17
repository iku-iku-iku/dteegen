#!/bin/bash

mkdir -p polyos && cd polyos
export OH_HOME=`pwd`
export OH_IMAGES=$OH_HOME/images
wget https://filebin.net/m6q51s538bgd12p1/images.tar.gz
tar -zxvf images.tar.gz

