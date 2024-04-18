#!/bin/bash
mkdir -p $MOUNT_PATH
sudo mount -o loop $OH_IMAGES/userdata.img $MOUNT_PATH
sudo cp "$PROJECT_PATH".generated/build/server $MOUNT_PATH
sudo cp "$PROJECT_PATH".generated/build/client $MOUNT_PATH
sudo cp "$PROJECT_PATH".generated/build/enclave.signed.so $MOUNT_PATH
sudo umount $MOUNT_PATH
echo "Finished"