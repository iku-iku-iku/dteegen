# dteegen

## quick start

### build OpenHarmony

If you don't want to build OpenHarmony on your own, you can follow the steps below.
```shell
mkdir polyos && cd polyos
export OH_HOME=`pwd`
wget https://filebin.net/69mg7qaaqfhu6yhd/chip_prod.img
wget https://filebin.net/69mg7qaaqfhu6yhd/Image
wget https://filebin.net/69mg7qaaqfhu6yhd/sys_prod.img
wget https://filebin.net/69mg7qaaqfhu6yhd/system.img
wget https://filebin.net/69mg7qaaqfhu6yhd/updater.img
wget https://filebin.net/69mg7qaaqfhu6yhd/userdata.img
wget https://filebin.net/69mg7qaaqfhu6yhd/vendor.img
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
git clone -b oh git@github.com:iku-iku-iku/Penglai-Enclave-sPMP.git
cd Penglai-Enclave-sPMP
./build_opensbi.sh
cp opensbi-1.2/build-oe/qemu-virt/platform/generic/firmware/fw_jump.bin $OH_HOME
# since we have installed penglai.ko in provisoned images, you can skip following steps
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
NOTE: this step is optional. Just for demonstrating that you can use cmake to build project locally without bothering with distribution and tee.
```shell
dteegen build $PROJECT_NAME
./new_project/build/insecure/client
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

```shell
sudo cp -r $OH_HOME/out/riscv64_virt/packages/phone/images{,1}
sudo cp -r $OH_HOME/out/riscv64_virt/packages/phone/images{,2}
```

### create scripts
create scripts for run server and client in the OH folder
```shell
cd $OH_HOME
cat > start_server.sh << EOF
#!/bin/bash

BR=br0
TAP=tap0

sudo tunctl -d $TAP
sudo tunctl -t $TAP -u $(whoami)
sudo ip link set $TAP up
sudo ip link set $TAP master $BR


root_path=$(pwd)
board=riscv64_virt
cpus=8
memory=8096
image_path=${root_path}/out/${board}/packages/phone/images2
QEMU=$(which qemu-system-riscv64)
sudo $QEMU \
    -name PolyOS-1 \
    -machine virt \
    -m ${memory} \
    -smp ${cpus} \
    -no-reboot \
    -netdev tap,id=net0,ifname=$TAP,script=no,downscript=no \
        -device virtio-net-device,netdev=net0,mac=12:22:33:44:55:66 \
    -drive if=none,file=${image_path}/updater.img,format=raw,id=updater,index=5 \
    -device virtio-blk-device,drive=updater \
    -drive if=none,file=${image_path}/system.img,format=raw,id=system,index=4 \
    -device virtio-blk-device,drive=system \
    -drive if=none,file=${image_path}/vendor.img,format=raw,id=vendor,index=3 \
    -device virtio-blk-device,drive=vendor \
    -drive if=none,file=${image_path}/userdata.img,format=raw,id=userdata,index=2 \
    -device virtio-blk-device,drive=userdata \
    -drive if=none,file=${image_path}/sys_prod.img,format=raw,id=sys-prod,index=1 \
    -device virtio-blk-device,drive=sys-prod \
    -drive if=none,file=${image_path}/chip_prod.img,format=raw,id=chip-prod,index=0 \
    -device virtio-blk-device,drive=chip-prod \
    -append "loglevel=1 ip=192.168.1.151:255.255.255.0::eth0:off sn=0023456789 console=tty0,115200 console=ttyS0,115200 init=/bin/init ohos.boot.hardware=virt root=/dev/ram0 rw ohos.required_mount.system=/dev/block/vdb@/usr@ext4@ro,barrier=1@wait,required ohos.required_mount.vendor=/dev/block/vdc@/vendor@ext4@ro,barrier=1@wait,required ohos.required_mount.sys_prod=/dev/block/vde@/sys_prod@ext4@ro,barrier=1@wait,required ohos.required_mount.chip_prod=/dev/block/vdf@/chip_prod@ext4@ro,barrier=1@wait,required ohos.required_mount.data=/dev/block/vdd@/data@ext4@nosuid,nodev,noatime,barrier=1,data=ordered,noauto_da_alloc@wait,reservedsize=1073741824 ohos.required_mount.misc=/dev/block/vda@/misc@none@none=@wait,required" \
    -kernel ${image_path}/Image \
    -initrd ${image_path}/ramdisk.img \
    -nographic \
    -device virtio-mouse-pci \
    -device virtio-keyboard-pci \
        -bios fw_jump.bin
    #-display sdl,gl=off

exit
EOF

cat > start_client.sh << EOF
#!/bin/bash

BR=br0
TAP=tap1

sudo tunctl -d $TAP
sudo tunctl -t $TAP -u $(whoami)
sudo ip link set $TAP up
sudo ip link set $TAP master $BR


root_path=$(pwd)
board=riscv64_virt
cpus=8
memory=8096
image_path=${root_path}/out/${board}/packages/phone/images1
QEMU=$(which qemu-system-riscv64)
sudo $QEMU \
    -name PolyOS-1 \
    -machine virt \
    -m ${memory} \
    -smp ${cpus} \
    -no-reboot \
    -netdev tap,id=net0,ifname=$TAP,script=no,downscript=no \
        -device virtio-net-device,netdev=net0,mac=12:22:33:44:55:67 \
    -drive if=none,file=${image_path}/updater.img,format=raw,id=updater,index=5 \
    -device virtio-blk-device,drive=updater \
    -drive if=none,file=${image_path}/system.img,format=raw,id=system,index=4 \
    -device virtio-blk-device,drive=system \
    -drive if=none,file=${image_path}/vendor.img,format=raw,id=vendor,index=3 \
    -device virtio-blk-device,drive=vendor \
    -drive if=none,file=${image_path}/userdata.img,format=raw,id=userdata,index=2 \
    -device virtio-blk-device,drive=userdata \
    -drive if=none,file=${image_path}/sys_prod.img,format=raw,id=sys-prod,index=1 \
    -device virtio-blk-device,drive=sys-prod \
    -drive if=none,file=${image_path}/chip_prod.img,format=raw,id=chip-prod,index=0 \
    -device virtio-blk-device,drive=chip-prod \
    -append "loglevel=1 ip=192.168.1.152:255.255.255.0::eth0:off sn=0023456789 console=tty0,115200 console=ttyS0,115200 init=/bin/init ohos.boot.hardware=virt root=/dev/ram0 rw ohos.required_mount.system=/dev/block/vdb@/usr@ext4@ro,barrier=1@wait,required ohos.required_mount.vendor=/dev/block/vdc@/vendor@ext4@ro,barrier=1@wait,required ohos.required_mount.sys_prod=/dev/block/vde@/sys_prod@ext4@ro,barrier=1@wait,required ohos.required_mount.chip_prod=/dev/block/vdf@/chip_prod@ext4@ro,barrier=1@wait,required ohos.required_mount.data=/dev/block/vdd@/data@ext4@nosuid,nodev,noatime,barrier=1,data=ordered,noauto_da_alloc@wait,reservedsize=1073741824 ohos.required_mount.misc=/dev/block/vda@/misc@none@none=@wait,required" \
    -kernel ${image_path}/Image \
    -initrd ${image_path}/ramdisk.img \
    -nographic \
    -device virtio-mouse-pci \
    -device virtio-keyboard-pci \
        -bios fw_jump.bin
    #-display sdl,gl=off

exit
EOF
```

### create bridge
```shell
sudo ip link add name br0 type bridge
sudo ip link set dev br0 up
sudo ip addr add 192.168.1.109/24 dev br0
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
