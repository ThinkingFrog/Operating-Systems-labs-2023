#!/usr/bin/env bash

mkdir build
cd build || exit
cmake ..
cmake --build .
cd ..
mv build/multithread .
rm -rf build
