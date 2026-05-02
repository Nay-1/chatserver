#!/bin/bash
set -e
set -x

mkdir -p build
rm -rf build/*
cd build
cmake ..
make -j 8   