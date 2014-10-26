#!/bin/sh
git config --global user.name "caoyilun"
git config --global user.email "caoyilun@sjtu.edu.cn"
git config --global color.status auto
git config --global color.branch auto

git config --global credential.helper cache
git config --global credential.helper 'cache --timeout=1'

git config --list
