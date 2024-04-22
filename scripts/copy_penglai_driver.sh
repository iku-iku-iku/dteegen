#!/bin/bash
sudo mount -o loop $OH_IMAGES/userdata.img $MOUNT_PATH
cp $PENGLAI_HOME/penglai-enclave-driver/penglai.ko $MOUNT_PATH
sudo umount $MOUNT_PATH
echo "Finished"