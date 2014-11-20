#!/bin/sh
cd ./Beignet-0.9.3-Source/
sudo rm -rf release
mkdir release
cd release

cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ../
#-D BUILD_EXAMPLES=ON
make -j4 all
sudo make install
