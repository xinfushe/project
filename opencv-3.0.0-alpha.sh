#!/bin/sh
sudo rm -rf opencv-3.0.0-alpha-build
mkdir opencv-3.0.0-alpha-build
cd opencv-3.0.0-alpha-build
mkdir release

cmake -E chdir ./release cmake -D CMAKE_BUILD_TYPE=release -D BUILD_EXAMPLES=ON -D CMAKE_INSTALL_PREFIX=/usr/local ../../opencv-3.0.0-alpha

cd release

make -j4 all

sudo make install
