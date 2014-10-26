#!/bin/sh
rm -rf ./project
mkdir project
cd project
touch helloworld.cpp
git init
git add .
git commit -m "Commit 0"
git log
