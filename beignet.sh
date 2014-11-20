#!/bin/sh
sudo rm -rf Beignet_build
mkdir Beignet_build
cd Beignet_build
#mkdir debug
mkdir release


#cmake -E chdir ./debug cmake -G "Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=/usr/local ../../Beignet-0.9.3-Source

cmake -E chdir ./release cmake -G "Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ../../Beignet-0.9.3-Source

#cd debug
#make -j4 all

cd release
make -j4 all
sudo make install
