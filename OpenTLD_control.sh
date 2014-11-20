#!/bin/sh
sudo rm -rf build_control
mkdir build_control
cd build_control
mkdir release
mkdir debug

cmake -E chdir ./release cmake -G "Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=release -D BUILD_QOPENTLD=ON -D USE_SYSTEM_LIBS=OFF ../../OpenTLD_control

cmake -E chdir ./debug cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=debug -D BUILD_QOPENTLD=ON -D USE_SYSTEM_LIBS=OFF ../../OpenTLD_control

#cmake -D CMAKE_BUILD_TYPE=Release -D BUILD_QOPENTLD=ON -D USE_SYSTEM_LIBS=OFF ../
#-D CMAKE_INSTALL_PREFIX=/usr/local

cd debug
make -j4 all
cd ../release
make -j4 all

