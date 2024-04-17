# dteegen

## quick start

It takes only 6 steps to get started!

1. Clone this repo.

```shell
git clone https://github.com/iku-iku-iku/dteegen.git
cd dteegen
```

2. Build OpenHarmony.

If you don't want to build OpenHarmony on your own, you can use the prebuilt images provided by us.

```shell
bash ./scripts/download_prebuilt.sh
```

If you want to build OpenHarmony on your own, you can follow the steps below.

```shell
# install dependencies
sudo apt install git-lfs
sudo apt install repo
sudo apt install qemu-system qemu
mkdir polyos && cd polyos
export OH_HOME=`pwd`
# download the source code
git config --global credential.helper 'cache --timeout=3600'
repo init -u https://isrc.iscas.ac.cn/gitlab/riscv/polyosmobile/ohos_qemu/manifest.git -b OpenHarmony-3.2-Release --no-repo-verify
repo sync -j$(nproc) -c && repo forall -c 'git lfs pull'
docker run -it --rm -v $(pwd):/polyos-mobile --workdir /polyos-mobile swr.cn-south-1.myhuaweicloud.com/openharmony-docker/openharmony-docker:1.0.0
bash build/prebuilts_download.sh
bash build.sh --product-name qemu_riscv64_virt_linux_system --ccache
export OH_IMAGES=$OH_HOME/out/riscv64_virt/packages/phone/images
```

3. Build opensbi and penglai driver.

```shell
git clone -b distributed-tee git@github.com:iku-iku-iku/Penglai-Enclave-sPMP.git
cd Penglai-Enclave-sPMP
export PENGLAI_HOME=`pwd`
# build opensbi
bash ./build_opensbi.sh
# build the driver
bash ./scripts/build_driver_for_oh.sh
```

4. Use dteegen for quick demo starter.

```shell
# download dteegen
curl -o dteegen https://raw.githubusercontent.com/iku-iku-iku/dteegen/master/scripts/all_in_one.sh
chmod +x dteegen
sudo mv dteegen /usr/local/bin
# create new project
export PROJECT_NAME=new_project
dteegen create $PROJECT_NAME

# build project locally
#**NOTE**: this step is optional. Just for demonstrating that you can use cmake to build project locally without bothering with distribution and tee.
#dteegen build $PROJECT_NAME
#./$PROJECT_NAME/build/insecure/client

# deploy project
# This step will generate a folder named new_project.generated and build the generated project.
dteegen deploy $PROJECT_NAME
# then you can see your target(e.g. client, server, enclave.signed.so) in path like new_project.generated/build
```


5. Do some preparation for running OpenHarmony. TIPS: you can copy paste the following commands to run altogether.

```shell
# copy opensbi to OH_HOME
cp $PENGLAI_HOME/opensbi-1.2/build-oe/qemu-virt/platform/generic/firmware/fw_jump.bin $OH_HOME
# copy scripts to OH_HOME
cp $PENGLAI_HOME/scripts/{start_server.sh,start_client.sh} $OH_HOME
# copy penglai driver to OH's /data
mkdir -p /tmp/mount
sudo mount -o loop $OH_IMAGES/userdata.img /tmp/mount
cp $PENGLAI_HOME/penglai-enclave-driver/penglai.ko /tmp/mount
sudo umount /tmp/mount
# inject depencies to OpenHarmony. NOTE: This step is optional since we have installed all libs in the provisoned images.
mkdir -p /tmp/mount
sudo mount -o loop $OH_IMAGES/system.img /tmp/mount
dteegen inject /tmp/mount
sudo umount /tmp/mount
# inject built files to OpenHarmony images
mkdir -p /tmp/mount
sudo mount -o loop $OH_IMAGES/userdata.img /tmp/mount
sudo cp $PROJECT_NAME.generated/build/{server,client,enclave.signed.so} /tmp/mount
sudo umount /tmp/mount
sudo mount -o loop $OH_IMAGES/system.img /tmp/mount
sudo cp $PROJECT_NAME.generated/build/lib/penglai/libpenglai_0.so /tmp/mount/system/lib64
sudo umount /tmp/mount
# Since instances can not share the same images, we need to copy them.
sudo cp -r "$OH_IMAGES"{,1}
sudo cp -r "$OH_IMAGES"{,2}
# create bridge
sudo ip link add name br0 type bridge
sudo ip link set dev br0 up
sudo ip addr add 192.168.1.109/24 dev br0
sudo iptables -P FORWARD ACCEPT
```

6. Run server and client.

```shell
# Tmux is recommended to run server and client separately. 
# --------------------------------
# run server with tee ability
cd $OH_HOME
./start_server.sh

cd data
insmod penglai.ko
export LD_LIBRARY_PATH=/lib64
./server

# --------------------------------
# run client without tee ability

cd $OH_HOME
./start_client.sh
cd data
export LD_LIBRARY_PATH=/lib64
./client

```
