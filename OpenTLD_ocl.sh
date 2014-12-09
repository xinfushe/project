#!/bin/sh
sudo rm -rf build
mkdir build
cd build
mkdir release
mkdir debug

cmake -E chdir ./release cmake -G "Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=release -D BUILD_QOPENTLD=ON -D USE_SYSTEM_LIBS=OFF ../../OpenTLD_ocl

cmake -E chdir ./debug cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=debug -D BUILD_QOPENTLD=ON -D USE_SYSTEM_LIBS=OFF ../../OpenTLD_ocl

#cmake -D CMAKE_BUILD_TYPE=Release -D BUILD_QOPENTLD=ON -D USE_SYSTEM_LIBS=OFF ../
#-D CMAKE_INSTALL_PREFIX=/usr/local

#make -j4
#sudo make install

