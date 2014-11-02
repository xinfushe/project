#!/bin/sh

cp -rf ./auto_focus_patchset/* ./OpenTLD
cd ./OpenTLD
patch -p1 < 0001-autofocus.patch
patch -p1 < 0002-autofocus2.patch
patch -p1 < 0003-autofocus2.patch
