#!/bin/sh
sudo rm -rf opencv-2.4.10.1-build
mkdir opencv-2.4.10.1-build
cd opencv-2.4.10.1-build
mkdir release

cmake -E chdir ./release cmake -G "Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=release -D WITH_QT=ON -D WITH_OPENCL=ON -D CMAKE_INSTALL_PREFIX=/usr/local ../../opencv-2.4.10.1

cd release

make -j4 all

sudo make install

#-D BUILD_EXAMPLES=ON
