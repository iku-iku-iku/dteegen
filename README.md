# dteegen

## quick start

### build OpenHarmony

If you don't want to build OpenHarmony on your own, you can follow the steps below.
```shell
mkdir polyos && cd polyos
export OH_HOME=`pwd`
wget https://filebin.net/69mg7qaaqfhu6yhd/Image
wget https://filebin.net/69mg7qaaqfhu6yhd/chip_prod.img
wget https://filebin.net/69mg7qaaqfhu6yhd/sys_prod.img
wget https://filebin.net/69mg7qaaqfhu6yhd/system.img
wget https://filebin.net/69mg7qaaqfhu6yhd/updater.img
wget https://filebin.net/69mg7qaaqfhu6yhd/userdata.img
wget https://filebin.net/69mg7qaaqfhu6yhd/vendor.img
wget https://filebin.net/m6q51s538bgd12p1/ramdisk.img
mkdir -p out/riscv64_virt/packages/phone/images
mv Image *.img out/riscv64_virt/packages/phone/images

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
# out/riscv64_virt/packages/phone/images

```

### build opensbi and penglai driver
```shell
git clone -b distributed-tee git@github.com:iku-iku-iku/Penglai-Enclave-sPMP.git
cd Penglai-Enclave-sPMP
./build_opensbi.sh
cp opensbi-1.2/build-oe/qemu-virt/platform/generic/firmware/fw_jump.bin $OH_HOME
# since we have installed penglai.ko in provisoned images, you can skip following steps
wget https://filebin.net/m6q51s538bgd12p1/oh-kernel.tar.gz
tar -zxvf oh-kernel.tar.gz
./docker_cmd.sh docker
./scripts/build_enclave_driver.sh
mkdir -p /tmp/mount
sudo mount -o loop $OH_HOME/out/riscv64_virt/packages/phone/images/userdata.img /tmp/mount
cp penglai-enclave-driver/penglai.ko /tmp/mount
sudo umount /tmp/mount
```

### install dteegen
```shell
curl -o dteegen https://raw.githubusercontent.com/iku-iku-iku/dteegen/master/scripts/all_in_one.sh
chmod +x dteegen
sudo mv dteegen /usr/local/bin
```

### create new project
```shell
PROJECT_NAME=new_project
dteegen create $PROJECT_NAME
```

### build project locally
**NOTE**: this step is optional. Just for demonstrating that you can use cmake to build project locally without bothering with distribution and tee.
```shell
dteegen build $PROJECT_NAME
./$PROJECT_NAME/build/insecure/client
```

### deploy project
This step will generate a folder named new_project.generated and build the generated project.
```shell
dteegen deploy $PROJECT_NAME
# then you can see your target(e.g. client, server, enclave.signed.so) in path like new_project.generated/build
```

### inject depencies to OpenHarmony
This step is optional since we have installed all libs in the provisoned images.
```shell
mkdir -p /tmp/mount
sudo mount -o loop $OH_HOME/out/riscv64_virt/packages/phone/images/system.img /tmp/mount
dteegen inject /tmp/mount
sudo umount /tmp/mount
```

### inject built files to OpenHarmony images
```shell
mkdir -p /tmp/mount
sudo mount -o loop $OH_HOME/out/riscv64_virt/packages/phone/images/userdata.img /tmp/mount
sudo cp $PROJECT_NAME.generated/build/server /tmp/mount
sudo cp $PROJECT_NAME.generated/build/client /tmp/mount
sudo cp $PROJECT_NAME.generated/build/enclave.signed.so /tmp/mount
sudo umount /tmp/mount
sudo mount -o loop $OH_HOME/out/riscv64_virt/packages/phone/images/system.img /tmp/mount
sudo cp $PROJECT_NAME.generated/build/lib/penglai/libpenglai_0.so /tmp/mount/system/lib64
sudo umount /tmp/mount

```

### copy images
Since instances can not share the same images, we need to copy them.
```shell
sudo cp -r $OH_HOME/out/riscv64_virt/packages/phone/images{,1}
sudo cp -r $OH_HOME/out/riscv64_virt/packages/phone/images{,2}
```

### create scripts
create scripts for run server and client in the OH folder
```shell
cd $OH_HOME
wget https://filebin.net/m6q51s538bgd12p1/start_client.sh
wget https://filebin.net/m6q51s538bgd12p1/start_server.sh
chmod +x start_client.sh start_server.sh
```

### create bridge
```shell
sudo ip link add name br0 type bridge
sudo ip link set dev br0 up
sudo ip addr add 192.168.1.109/24 dev br0
sudo iptables -P FORWARD ACCEPT
```

### server

run server with tee ability

```shell
./start_server.sh

cd data
insmod penglai.ko
export LD_LIBRARY_PATH=/lib64
./server

```

### client

run client without tee ability

```shell
./start_client.sh
cd data
export LD_LIBRARY_PATH=/lib64
./client

```
