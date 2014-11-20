#!/bin/sh

sudo su
cd /etc
echo "/usr/local/lib" >> ld.so.conf
ldconfig

然后编辑/etc/bash.bashrc

加入

PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
export PKG_CONFIG_PATH

sudo source /etc/bash.bashrc

`pkg-config --cflags --libs opencv`
