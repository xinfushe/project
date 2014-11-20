#!/bin/sh
cd ~/Desktop/opencv-2.4.9/
sudo rm -rf release
mkdir release
cd release

sudo cmake -D CMAKE_BUILD_TYPE=Release -D BUILD_EXAMPLES=ON -D CMAKE_INSTALL_PREFIX=/usr/local ../
#-D OPENCV_EXTRA_MODULES_PATH=../opencv_contrib/modules 
sudo make -j4
sudo make install
