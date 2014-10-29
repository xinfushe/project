#!/bin/sh
cd ~/Desktop/Beignet-0.9.3-Source/
rm -r release
mkdir release
cd release

cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ../
#-D BUILD_EXAMPLES=ON
make -j4
sudo make install
