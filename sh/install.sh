#!/bin/sh
sudo apt-get autoclean
sudo apt-get autoremove

sudo apt-get install gksu vim cmake g++ build-essential valgrind
#beignet
sudo apt-get install libdrm-dev libxext6 libxext-dev libxfixes-dev libgl1-mesa-dev-lts-trusty libegl1-mesa-dev libedit-dev ocl-icd-dev ocl-icd-libopencl1
#opencv
sudo apt-get install libgtk2.0-dev
#如果只是用来运行OpenCV程序，仅需安装libcv1，libcvaux1，libhighgui1：
sudo apt-get install libcv2.4 libcvaux2.4 libhighgui2.4
#如果你要使用OpenCV来编写程序，那么还需要安装libcv-dev，libcvaux-dev，libhighgui-dev包。
sudo apt-get install libcv-dev libcvaux-dev libhighgui-dev
# libavcodec-dev libavformat-dev libjpeg62-dev libtiff4-dev libswscale-dev libjasper-dev
#build-essential cmake libgtk2.0-dev pkg-config python-dev python-numpy libavcodec-dev libavformat-dev libswscale-dev  
#libv4l是直接用来捕获摄像头的库，只有有了这些，在opencv重新编译后，才能够将cvCaptureFromCAM于真正的设备连接，从而获取视频
sudo apt-get install libv4l-dev libv4l-0 v4l-utils

sudo apt-get install git git-core gitk git-gui

sudo apt-get install libqt4-dev

sudo apt-get install libva-dev

sudo apt-get install eclipse eclipse-cdt

sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo apt-get autoclean
sudo apt-get autoremove
