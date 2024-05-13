#!/bin/bash
cd "${0%/*}"
cd ..

mkdir -p build
mkdir -p build/unix
cd build/unix

cmake -DSDL_STATIC=ON -DSDL_SHARED=OFF -S ../..
cmake --build ./ --target install --config Release --parallel

cpack -C Release
