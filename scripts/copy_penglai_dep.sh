#!/bin/bash
mkdir -p $MOUNT_PATH
sudo mount -o loop $OH_IMAGES/system.img $MOUNT_PATH
dteegen inject $MOUNT_PATH $PROJECT_PATH
sudo umount $MOUNT_PATH
echo "Finished"