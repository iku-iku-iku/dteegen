#!/bin/bash
sudo rm -rf "$OH_IMAGES"{1,2}
sudo cp -r "$OH_IMAGES"{,1}
sudo cp -r "$OH_IMAGES"{,2}
echo "Finished"