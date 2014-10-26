#!/bin/sh
cd ./OpenTLD/
rm -r release
mkdir release
cd release

cmake -D CMAKE_BUILD_TYPE=Release -D BUILD_QOPENTLD=ON -D USE_SYSTEM_LIBS=OFF ../
#-D CMAKE_INSTALL_PREFIX=/usr/local

make -j4
#sudo make install
