#!/bin/sh

cd ~/Desktop/

sudo rm -rf ./compare
mkdir compare

cd ~/Desktop/project-repo/
sh OpenTLD_ocl.sh
cd ./build/release/
make -j4 all
cp -rf ./bin/* ~/Desktop/compare
#cd ../debug/
#make -j4 all
cd ~/Desktop/compare

cd ~/Desktop/project-repo/
sh OpenTLD_cuda_removed.sh
cd ./build_cuda_removed/release
make -j4 all
cp -rf ./bin/*opentld ~/Desktop/compare
cd ~/Desktop/compare
mv -f opentld opentld_cpu
mv -f qopentld qopentld_cpu

